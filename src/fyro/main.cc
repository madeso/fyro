#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>

#include <nlohmann/json.hpp>
#include "physfs.h"

#include "fyro/fyro.h"
#include "lox/lox.h"
#include "lox/printhandler.h"

using json = nlohmann::json;
namespace fs = std::filesystem;


struct Exception
{
	std::vector<std::string> errors;

	Exception append(const std::string& str)
	{
		errors.emplace_back(str);
		return *this;
	}

	Exception append(const std::vector<std::string>& li)
	{
		for(const auto& str: li)
		{
			errors.emplace_back(str);
		}
		return *this;
	}
};

Exception collect_exception()
{
	try
	{
		throw;
	}
	catch(const Exception& e)
	{
		return e;
	}
	catch(const std::exception& e)
	{
		return {{e.what()}};
	}
	catch(...)
	{
		return {{"unknown errror"}};
	}
}


std::string get_physfs_error()
{
	const auto code = PHYSFS_getLastErrorCode();
	return PHYSFS_getErrorByCode(code);
}

Exception physfs_exception(const std::string& message)
{
	const std::string error = get_physfs_error();
	return Exception {{message + error}};
}

static PHYSFS_EnumerateCallbackResult physfs_callback(void *data, const char *origdir, const char *fname)
{
	auto* ret = static_cast<std::vector<std::string>*>(data);
	ret->emplace_back(std::string(origdir) + fname);

	// std::cout << "file " << fname << " in " << origdir << "\n";
	return PHYSFS_ENUM_OK;
}

std::vector<std::string> physfs_files_in_dir(const std::string& dir)
{
	std::vector<std::string> r;
	auto ok = PHYSFS_enumerate(dir.c_str(), &physfs_callback, &r);

	if(ok == 0)
	{
		std::cerr << "failed to enumerate " << dir << "\n";
	}

	return r;
}

std::string read_file_to_string(const std::string& path)
{
	auto* file = PHYSFS_openRead(path.c_str());
	if(file == nullptr)
	{
		// todo(Gustav): split path and find files in dir
		throw physfs_exception("unable to open file").append("found files").append(physfs_files_in_dir(""));
	}

	std::vector<char> ret;
	while(PHYSFS_eof(file) == 0)
	{
		char buffer[1024];
		const auto read = PHYSFS_readBytes(file, buffer, 1024);
		if(read > 0)
		{
			ret.insert(ret.end(), buffer, buffer+read);
		}
	}

	ret.emplace_back('\0');

	PHYSFS_close(file);
	return ret.data();
}


json load_json(const std::string& path)
{
	const auto src = read_file_to_string(path);
	return json::parse(src);
}


struct GameData
{
	std::string title = "fyro";
	int width = 800;
	int height = 600;
};

GameData load_game_data(const std::string& path)
{
	try
	{
		json data = load_json(path);

		auto r = GameData{};
		r.title  = data["title"] .get<std::string>();
		r.width  = data["width"] .get<int>();
		r.height = data["height"].get<int>();
		return r;
	}
	catch(...)
	{
		auto x = collect_exception();
		x.errors.push_back("while reading " + path);
		throw x;
	}
}

struct PrintLoxError : lox::PrintHandler
{
	void on_line(std::string_view line) override
	{
		std::cerr << line << "\n";
	}
};

struct Rgb { float r; float g; float b; };

struct RenderData
{
	const render::RenderCommand& rc;
	std::optional<render::RenderLayer2> layer;

	explicit RenderData(const render::RenderCommand& rr) : rc(rr) {}
	~RenderData()
	{
		if(layer)
		{
			layer->batch->submit();
		}
	}
};

struct RenderArg
{
	std::shared_ptr<RenderData> data;
};

struct State
{
	State() = default;
	virtual ~State() = default;
	virtual void update(float) = 0;
	virtual void render(const render::RenderCommand& rc) = 0;
};

struct ScriptState : State
{
	std::shared_ptr<lox::Instance> instance;
	lox::Lox* lox;

	std::shared_ptr<lox::Callable> on_update;
	std::shared_ptr<lox::Callable> on_render;

	ScriptState(std::shared_ptr<lox::Instance> in, lox::Lox* lo)
		: instance(in)
		, lox(lo)
	{
		on_update = instance->get_bound_method_or_null("update");
		on_render = instance->get_bound_method_or_null("render");
	}

	void update(float dt) override
	{
		if(on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(const render::RenderCommand& rc) override
	{
		if(on_render)
		{
			auto ra = std::make_shared<RenderData>(rc);
			auto render = lox->make_native<RenderArg>(RenderArg{ra});
			on_render->call({{render}});
			// todo(Gustav): clean up render object here so calling functions on it can crash nicely instead of going down in assert/crash hell
		}
	}
};

struct ExampleGame : public Game
{
	lox::Lox lox;

	std::unique_ptr<State> next_state;
	std::unique_ptr<State> state;

	ExampleGame()
		: lox(std::make_unique<PrintLoxError>(), [](const std::string& str)
		{
			std::cout << str << "\n";
		})
	{
		bind_functions();
	}

	void run_main()
	{
		const auto src = read_file_to_string("main.lox");
		if(false == lox.run_string(src))
		{
			throw Exception{{"Unable to run script"}};
		}
	}

	void bind_functions()
	{
		auto fyro = lox.in_package("fyro");
		
		fyro->define_native_function("set_state", [this](lox::Callable*, lox::ArgumentHelper& arguments)
		{
			auto instance = arguments.require_instance();
			arguments.complete();
			next_state = std::make_unique<ScriptState>(instance, &lox);
			return lox::make_nil();
		});

		fyro->define_native_class<Rgb>("Rgb", [](lox::ArgumentHelper& ah) -> Rgb
		{
			const auto r = static_cast<float>(ah.require_float());
			const auto g = static_cast<float>(ah.require_float());
			const auto b = static_cast<float>(ah.require_float());
			ah.complete();
			return Rgb{r, g, b};
		});

		// todo(Gustav): validate argumends and raise script error on invalid
		fyro->define_native_class<RenderArg>("RenderCommand")
			.add_function("windowbox", [](RenderArg& r, lox::ArgumentHelper& ah)
			{
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());
				ah.complete();

				if(width <= 0.0f) { lox::raise_error("width must be positive"); }
				if(height <= 0.0f) { lox::raise_error("height must be positive"); }
				if(r.data == nullptr) { lox::raise_error("must be called inside State.render()"); }

				r.data->layer = render::with_layer2(r.data->rc, render::LayoutData{render::ViewportStyle::black_bars, width, height});
				return lox::make_nil();
			})
			.add_function("clear", [](RenderArg& r, lox::ArgumentHelper& ah)
			{
				auto color = ah.require_native<Rgb>();
				ah.complete();
				if(r.data == nullptr) { lox::raise_error("must be called inside State.render()"); }
				if(r.data->layer.has_value() == false) { lox::raise_error("need to setup virtual render area first"); }

				r.data->layer->batch->quad({}, r.data->layer->viewport_aabb_in_worldspace, {}, {color->r, color->g, color->b, 1.0f});
				return lox::make_nil();
			})
			.add_function("rect", [](RenderArg& r, lox::ArgumentHelper& ah)
			{
				auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());

				ah.complete();

				if(width <= 0.0f) { lox::raise_error("width must be positive"); }
				if(height <= 0.0f) { lox::raise_error("height must be positive"); }
				if(r.data == nullptr) { lox::raise_error("must be called inside State.render()"); }
				if(r.data->layer.has_value() == false) { lox::raise_error("need to setup virtual render area first"); }

				r.data->layer->batch->quad({},
					Rect{width, height}.translate(x, y),
					{}, {color->r, color->g, color->b, 1.0f}
				);
				return lox::make_nil();
			})
		;
	}

	void
	on_update(float dt) override
	{
		if(state)
		{
			state->update(dt);
		}
		if(next_state != nullptr)
		{
			state = std::move(next_state);
		}
	}

	void
	on_render(const render::RenderCommand& rc) override
	{
		if(state)
		{
			state->render(rc);
		}
		else
		{
			// todo(Gustav): draw some basic thing here since to state is set?
			auto r = render::with_layer2(rc, render::LayoutData{render::ViewportStyle::extended, 200.0f, 200.0f});
			r.batch->quad({}, r.viewport_aabb_in_worldspace, {}, {0.8, 0.8, 0.8, 1.0f});
			r.batch->submit();
		}
	}

	void on_mouse_position(const render::InputCommand&, const glm::ivec2&) override
	{
	}
};


struct Physfs
{
	Physfs(const Physfs&) = delete;
	Physfs(Physfs&&) = delete;
	Physfs operator=(const Physfs&) = delete;
	Physfs operator=(Physfs&&) = delete;

	Physfs(const std::string& path)
	{
		const auto ok = PHYSFS_init(path.c_str());
		if(ok == 0)
		{
			throw physfs_exception("unable to init");
		}
	}

	~Physfs()
	{
		const auto ok = PHYSFS_deinit();
		if(ok == 0)
		{
			const std::string error = get_physfs_error();
			std::cerr << "Unable to shut down physfs: " << error << "\n";
		}
	}

	void setup(const std::string& root)
	{
		std::cout << "sugggested root is: " << root << "\n";
		PHYSFS_mount(root.c_str(), "/", 1);
	}
};


int run(int argc, char** argv)
{
	auto physfs = Physfs(argv[0]);
	
	if(argc == 2)
	{
		fs::path folder = argv[1];
		folder = fs::canonical(folder);
		physfs.setup(folder.c_str());
	}
	else
	{
		physfs.setup(PHYSFS_getBaseDir());
	}

	const auto data = load_game_data("main.json");

	return run_game
	(
		data.title, glm::ivec2{data.width, data.height}, false, []()
		{
			auto game = std::make_shared<ExampleGame>();
			game->run_main();
			return game;
		}
	);
}


int
main(int argc, char** argv)
{
	try
	{
		return run(argc, argv);
	}
	catch(...)
	{
		auto x = collect_exception();
		for(const auto& e: x.errors)
		{
			std::cerr << e << '\n';
		}
		return -1;
	}
}
