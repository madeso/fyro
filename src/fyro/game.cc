#include "fyro/game.h"

#include <random>
#include "lox/printhandler.h"

#include "fyro/collision2.h"
#include "fyro/tiles.h"
#include "fyro/io.h"
#include "fyro/log.h"
#include "fyro/input.h"
#include "fyro/exception.h"
#include "fyro/vfs.h"
#include "fyro/rgb.h"
#include "fyro/gamedata.h"
#include "fyro/sprite.h"
#include "fyro/log.h"

int to_int(lox::Ti ti)
{
	return static_cast<int>(ti);
}

struct RenderData
{
	const render::RenderCommand& rc;
	std::optional<render::RenderLayer2> layer;
	glm::vec2 focus;

	explicit RenderData(const render::RenderCommand& rr)
		: rc(rr)
		, focus(0, 0)
	{
	}

	~RenderData()
	{
		if (layer)
		{
			layer->batch->submit();
		}
	}
};

struct RenderArg
{
	std::shared_ptr<RenderData> data;
};

struct ScriptPlayer
{
	std::shared_ptr<Player> player;
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
		if (on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(const render::RenderCommand& rc) override
	{
		if (on_render)
		{
			auto ra = std::make_shared<RenderData>(rc);
			auto render = lox->make_native<RenderArg>(RenderArg{ra});
			on_render->call({{render}});
			// todo(Gustav): clean up render object here so calling functions on it can crash nicely instead of going down in assert/crash hell
		}
	}
};

struct ScriptRandom
{
	using Engine = std::mt19937;

	Engine generator;

	static Engine create_engine()
	{
		std::random_device seeder;
		Engine generator(seeder());
		return generator;
	}

	ScriptRandom()
		: generator(create_engine())
	{
	}

	lox::Ti next_int(lox::Ti max)
	{
		std::uniform_int_distribution<lox::Ti> distribution(0, max - 1);
		return distribution(generator);
	}
};

struct ScriptFont
{
	std::shared_ptr<render::Font> font;

	void setup(const std::string& path, float height)
	{
		auto bytes = read_file_to_bytes(path);
		font = std::make_shared<render::Font>(
			reinterpret_cast<const unsigned char*>(bytes.data()), height
		);
	}
};

std::shared_ptr<render::Texture> load_texture(const std::string& path)
{
	const auto bytes = read_file_to_bytes(path);
	return std::make_shared<render::Texture>(render::load_image_from_bytes(
		reinterpret_cast<const unsigned char*>(bytes.data()),
		static_cast<int>(bytes.size()),
		render::TextureEdge::repeat,
		render::TextureRenderStyle::pixel,
		render::Transparency::include
	));
}

// c++ wrappers over the specific lox class

struct ScriptActor
{
	std::shared_ptr<lox::Instance> instance;

	std::shared_ptr<lox::Callable> on_update;
	std::shared_ptr<lox::Callable> on_render;
	std::shared_ptr<lox::Callable> on_get_squished;

	ScriptActor(std::shared_ptr<lox::Instance> in)
		: instance(in)
	{
		on_update = instance->get_bound_method_or_null("update");
		on_render = instance->get_bound_method_or_null("render");
		on_get_squished = instance->get_bound_method_or_null("get_squished");
	}

	void update(float dt)
	{
		if (on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(std::shared_ptr<lox::Object> rc)
	{
		if (on_render)
		{
			on_render->call({{rc}});
		}
	}

	bool is_riding_solid(fyro::Solid*)
	{
		// todo(Gustav): implement this
		return false;
	}

	void get_squished()
	{
		if (on_get_squished)
		{
			on_get_squished->call({{}});
		}
	}
};

struct ScriptSolid
{
	std::shared_ptr<lox::Instance> instance;

	std::shared_ptr<lox::Callable> on_update;
	std::shared_ptr<lox::Callable> on_render;

	ScriptSolid(std::shared_ptr<lox::Instance> in)
		: instance(in)
	{
		on_update = instance->get_bound_method_or_null("update");
		on_render = instance->get_bound_method_or_null("render");
	}

	void update(float dt)
	{
		if (on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(std::shared_ptr<lox::Object> rc)
	{
		if (on_render)
		{
			on_render->call({{rc}});
		}
	}
};

// the actual c++ class dispatching the virtual functions to the base

struct ScriptActorImpl : fyro::Actor
{
	std::shared_ptr<ScriptActor> dispatcher;

	bool is_riding_solid(fyro::Solid* solid) override
	{
		if (dispatcher)
		{
			return dispatcher->is_riding_solid(solid);
		}
		else
		{
			return false;
		}
	}

	void get_squished() override
	{
		if (dispatcher)
		{
			return dispatcher->get_squished();
		}
	}

	void update(float dt) override
	{
		assert(dispatcher);
		dispatcher->update(dt);
	}

	void render(std::shared_ptr<lox::Object> rc) override
	{
		assert(dispatcher);
		dispatcher->render(rc);
	}
};

struct ScriptSolidImpl : fyro::Solid
{
	std::shared_ptr<ScriptSolid> dispatcher;

	void update(float dt) override
	{
		assert(dispatcher);
		dispatcher->update(dt);
	}

	void render(std::shared_ptr<lox::Object> rc) override
	{
		assert(dispatcher);
		dispatcher->render(rc);
	}
};

// the lox variable wrapper

struct ScriptActorBase
{
	ScriptActorBase()
		: impl(std::make_shared<ScriptActorImpl>())
	{
	}

	std::shared_ptr<ScriptActorImpl> impl;
};

struct ScriptSolidBase
{
	ScriptSolidBase()
		: impl(std::make_shared<ScriptSolidImpl>())
	{
	}

	std::shared_ptr<ScriptSolidImpl> impl;
};

struct ScriptLevelData
{
	fyro::Level level;
	Map tiles;
};

struct FixedSolid : fyro::Solid
{
	void update(float) override
	{
	}

	void render(std::shared_ptr<lox::Object>) override
	{
	}
};

struct ScriptLevel
{
	ScriptLevel()
		: data(std::make_shared<ScriptLevelData>())
	{
	}

	std::shared_ptr<ScriptLevelData> data;

	void load_tmx(const std::string& path)
	{
		auto source = read_file_to_string(path);
		tmx::Map map;
		const auto was_loaded = map.loadFromString(source, get_dir_from_file(path));
		if (was_loaded == false)
		{
			throw Exception{{"failed to parse tmx file"}};
		}
		data->tiles.load_from_map(map);

		for (const auto& rect: data->tiles.get_collisions())
		{
			auto solid = std::make_shared<FixedSolid>();
			solid->level = &data->level;

			solid->position
				= glm::ivec2{static_cast<int>(rect.left), static_cast<int>(rect.bottom)};
			solid->size
				= Recti{static_cast<int>(rect.get_width()), static_cast<int>(rect.get_height())};

			data->level.solids.emplace_back(solid);
		}
	}

	void add_actor(std::shared_ptr<lox::Instance> x)
	{
		auto dispatcher = std::make_shared<ScriptActor>(x);
		auto actor = lox::get_derived<ScriptActorBase>(x);
		actor->impl->dispatcher = dispatcher;
		actor->impl->level = &data->level;
		data->level.actors.emplace_back(actor->impl);
	}

	void add_solid(std::shared_ptr<lox::Instance> x)
	{
		auto dispatcher = std::make_shared<ScriptSolid>(x);
		auto solid = lox::get_derived<ScriptSolidBase>(x);
		solid->impl->dispatcher = dispatcher;
		solid->impl->level = &data->level;
		data->level.solids.emplace_back(solid->impl);
	}
};

struct PrintLoxError : lox::PrintHandler
{
	void on_line(std::string_view line) override
	{
		LOG_ERROR("> {0}", line);
	}
};

namespace script
{
void look_at(RenderData* r_data, ScriptLevelData* level_data, float focusx, float focusy)
{
	const bool pixelize = true;
	using TPixelize = int;

	const auto& aabb = r_data->layer->viewport_aabb_in_worldspace;
	const auto center_screen = glm::vec3{0.5f * aabb.get_width(), 0.5f * aabb.get_height(), 0};

	if (auto bounds = level_data->tiles.get_bounds(); bounds)
	{
		focusx = std::max(focusx, bounds->left + center_screen.x);
		focusx = std::min(focusx, bounds->right - center_screen.x);

		focusy = std::min(focusy, bounds->top - center_screen.y);
		focusy = std::max(focusy, bounds->bottom + center_screen.y);
	}

	const auto translation_float = glm::vec3{-focusx, -focusy, 0} + center_screen;
	const auto translation = pixelize
		? glm::vec3
		{
			static_cast<TPixelize>(translation_float.x),
			static_cast<TPixelize>(translation_float.y),
			0
		}
		: translation_float;
	const auto camera_matrix = glm::translate(glm::mat4(1.0f), translation);
	r_data->rc.set_camera(camera_matrix);
	const auto focus_float = glm::vec2{focusx, focusy} - glm::vec2(center_screen);
	r_data->focus
		= pixelize
			? glm::
				  vec2{static_cast<TPixelize>(focus_float.x), static_cast<TPixelize>(focus_float.y)}
			: focus_float;
}
}  //  namespace script

ExampleGame::ExampleGame()
	: lox(std::make_unique<PrintLoxError>(), [](const std::string& str) { LOG_INFO("> {0}", str); })
	, texture_cache([](const std::string& path) { return load_texture(path); })
{
	// todo(Gustav): read/write to json and provide ui for adding new mappings
	keyboards.mappings.emplace_back(create_default_mapping_for_player1());
	input.add_keyboard(std::make_shared<InputDevice_Keyboard>(&keyboards, 0));

	bind_named_colors();
	bind_collision();
	bind_functions();
}

void ExampleGame::on_imgui()
{
	for (auto& f: loaded_fonts)
	{
		f->imgui();
	}
	input.on_imgui();
}

void ExampleGame::run_main()
{
	const auto src = read_file_to_string("main.lox");
	if (false == lox.run_string(src))
	{
		throw Exception{{"Unable to run script"}};
	}
}

void ExampleGame::bind_named_colors()
{
	auto rgb = lox.in_package("fyro.rgb");
	rgb->add_native_getter("white", [&]() { return lox.make_native(Rgb{255, 255, 255}); });
	rgb->add_native_getter("light_gray", [&]() { return lox.make_native(Rgb{160, 160, 160}); });
	rgb->add_native_getter("gray", [&]() { return lox.make_native(Rgb{127, 127, 127}); });
	rgb->add_native_getter("dark_gray", [&]() { return lox.make_native(Rgb{87, 87, 87}); });
	rgb->add_native_getter("black", [&]() { return lox.make_native(Rgb{0, 0, 0}); });
	rgb->add_native_getter("red", [&]() { return lox.make_native(Rgb{173, 35, 35}); });
	rgb->add_native_getter("pure_red", [&]() { return lox.make_native(Rgb{255, 0, 0}); });
	rgb->add_native_getter("blue", [&]() { return lox.make_native(Rgb{42, 75, 215}); });
	rgb->add_native_getter("pure_blue", [&]() { return lox.make_native(Rgb{0, 0, 255}); });
	rgb->add_native_getter("light_blue", [&]() { return lox.make_native(Rgb{157, 175, 255}); });
	rgb->add_native_getter("normal_blue", [&]() { return lox.make_native(Rgb{127, 127, 255}); });
	rgb->add_native_getter(
		"cornflower_blue", [&]() { return lox.make_native(Rgb{100, 149, 237}); }
	);
	rgb->add_native_getter("green", [&]() { return lox.make_native(Rgb{29, 105, 20}); });
	rgb->add_native_getter("pure_green", [&]() { return lox.make_native(Rgb{0, 255, 0}); });
	rgb->add_native_getter("light_green", [&]() { return lox.make_native(Rgb{129, 197, 122}); });
	rgb->add_native_getter("yellow", [&]() { return lox.make_native(Rgb{255, 238, 51}); });
	rgb->add_native_getter("pure_yellow", [&]() { return lox.make_native(Rgb{255, 255, 0}); });
	rgb->add_native_getter("orange", [&]() { return lox.make_native(Rgb{255, 146, 51}); });
	rgb->add_native_getter("pure_orange", [&]() { return lox.make_native(Rgb{255, 127, 0}); });
	rgb->add_native_getter("brown", [&]() { return lox.make_native(Rgb{129, 74, 25}); });
	rgb->add_native_getter("pure_brown", [&]() { return lox.make_native(Rgb{250, 75, 0}); });
	rgb->add_native_getter("purple", [&]() { return lox.make_native(Rgb{129, 38, 192}); });
	rgb->add_native_getter("pure_purple", [&]() { return lox.make_native(Rgb{128, 0, 128}); });
	rgb->add_native_getter("pink", [&]() { return lox.make_native(Rgb{255, 205, 243}); });
	rgb->add_native_getter("pure_pink", [&]() { return lox.make_native(Rgb{255, 192, 203}); });
	rgb->add_native_getter("pure_beige", [&]() { return lox.make_native(Rgb{245, 245, 220}); });
	rgb->add_native_getter("tan", [&]() { return lox.make_native(Rgb{233, 222, 187}); });
	rgb->add_native_getter("pure_tan", [&]() { return lox.make_native(Rgb{210, 180, 140}); });
	rgb->add_native_getter("cyan", [&]() { return lox.make_native(Rgb{41, 208, 208}); });
	rgb->add_native_getter("pure_cyan", [&]() { return lox.make_native(Rgb{0, 255, 255}); });
}

void ExampleGame::bind_collision()
{
	auto fyro = lox.in_package("fyro");

	lox.in_global_scope()
		->define_native_class<ScriptActorBase>("Actor")
		.add_function(
			"move_x",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float());
				ah.complete();
				auto r = x.impl->move_x(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			}
		)
		.add_function(
			"move_y",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float());
				ah.complete();
				auto r = x.impl->move_y(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			}
		)
		.add_property<lox::Ti>(
			"x",
			[](ScriptActorBase& x) -> lox::Ti { return x.impl->position.x; },
			[](ScriptActorBase& x, lox::Ti v) { x.impl->position.x = to_int(v); }
		)
		.add_property<lox::Ti>(
			"y",
			[](ScriptActorBase& x) -> lox::Ti { return x.impl->position.y; },
			[](ScriptActorBase& x, lox::Ti v) { x.impl->position.y = to_int(v); }
		)
		.add_getter<lox::Ti>(
			"width", [](ScriptActorBase& x) -> lox::Ti { return x.impl->size.get_width(); }
		)
		.add_getter<lox::Ti>(
			"height", [](ScriptActorBase& x) -> lox::Ti { return x.impl->size.get_height(); }
		)
		.add_function(
			"set_size",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto width = to_int(ah.require_int());
				auto height = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{width, height};
				return lox::make_nil();
			}
		)
		.add_function(
			"set_lrud",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto left = to_int(ah.require_int());
				auto right = to_int(ah.require_int());
				auto up = to_int(ah.require_int());
				auto down = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{left, down, right, up};
				return lox::make_nil();
			}
		);

	lox.in_global_scope()
		->define_native_class<ScriptSolidBase>("Solid")
		.add_function(
			"move",
			[](ScriptSolidBase& s, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dx = static_cast<float>(ah.require_float());
				auto dy = static_cast<float>(ah.require_float());
				ah.complete();
				s.impl->Move(dx, dy);
				return lox::make_nil();
			}
		)
		.add_property<lox::Ti>(
			"x",
			[](ScriptSolidBase& x) -> lox::Ti { return x.impl->position.x; },
			[](ScriptSolidBase& x, lox::Ti v) { x.impl->position.x = to_int(v); }
		)
		.add_property<lox::Ti>(
			"y",
			[](ScriptSolidBase& x) -> lox::Ti { return x.impl->position.y; },
			[](ScriptSolidBase& x, lox::Ti v) { x.impl->position.y = to_int(v); }
		)
		.add_getter<lox::Ti>(
			"width", [](ScriptSolidBase& x) -> lox::Ti { return x.impl->size.get_width(); }
		)
		.add_getter<lox::Ti>(
			"height", [](ScriptSolidBase& x) -> lox::Ti { return x.impl->size.get_height(); }
		)
		.add_function(
			"set_size",
			[](ScriptSolidBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto width = to_int(ah.require_int());
				auto height = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{width, height};
				return lox::make_nil();
			}
		);

	fyro->define_native_class<ScriptLevel>("Level")
		.add_function(
			"add",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto inst = ah.require_instance();
				ah.complete();
				r.add_actor(inst);
				return lox::make_nil();
			}
		)
		.add_function(
			"load_tmx",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto path = ah.require_string();
				ah.complete();
				r.load_tmx(path);
				return lox::make_nil();
			}
		)
		.add_function(
			"add_solid",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto inst = ah.require_instance();
				ah.complete();
				r.add_solid(inst);
				return lox::make_nil();
			}
		)
		.add_function(
			"update",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dt = static_cast<float>(ah.require_float());
				ah.complete();
				r.data->level.update(dt);
				return lox::make_nil();
			}
		)
		.add_function(
			"render",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto rend = ah.require_native<RenderArg>();
				ah.complete();
				r.data->tiles.render(
					*rend->data->layer->batch, rend->data->layer->viewport_aabb_in_worldspace
				);
				r.data->level.render(rend.instance);
				return lox::make_nil();
			}
		);
}

void ExampleGame::bind_functions()
{
	auto fyro = lox.in_package("fyro");

	fyro->define_native_function(
		"set_state",
		[this](lox::Callable*, lox::ArgumentHelper& arguments)
		{
			auto instance = arguments.require_instance();
			arguments.complete();
			next_state = std::make_unique<ScriptState>(instance, &lox);
			return lox::make_nil();
		}
	);

	fyro->define_native_function(
		"get_input",
		[this](lox::Callable*, lox::ArgumentHelper& arguments)
		{
			arguments.complete();
			auto frame = input.capture_player();
			return lox.make_native<ScriptPlayer>(ScriptPlayer{frame});
		}
	);

	fyro->define_native_class<Rgb>(
		"Rgb",
		[](lox::ArgumentHelper& ah) -> Rgb
		{
			const auto r = static_cast<float>(ah.require_float());
			const auto g = static_cast<float>(ah.require_float());
			const auto b = static_cast<float>(ah.require_float());
			ah.complete();
			return Rgb{r, g, b};
		}
	);

	fyro->define_native_class<glm::ivec2>(
		"vec2i",
		[](lox::ArgumentHelper& ah) -> glm::ivec2
		{
			const auto x = ah.require_int();
			const auto y = ah.require_int();
			ah.complete();
			return glm::ivec2{x, y};
		}
	);

	// todo(Gustav): validate argumends and raise script error on invalid
	fyro->define_native_class<RenderArg>("RenderCommand")
		.add_function(
			"windowbox",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());
				ah.complete();

				if (width <= 0.0f)
				{
					lox::raise_error("width must be positive");
				}
				if (height <= 0.0f)
				{
					lox::raise_error("height must be positive");
				}
				if (r.data == nullptr)
				{
					lox::raise_error("must be called inside State.render()");
					return nullptr;
				}

				r.data->layer = render::with_layer2(
					r.data->rc, render::LayoutData{render::ViewportStyle::black_bars, width, height}
				);
				return lox::make_nil();
			}
		)
		.add_function(
			"look_at",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto focusx = static_cast<float>(ah.require_float());
				auto focusy = static_cast<float>(ah.require_float());
				auto level = ah.require_native<ScriptLevel>();
				ah.complete();

				if (r.data == nullptr)
				{
					lox::raise_error("must be called inside State.render()");
					return nullptr;
				}

				script::look_at(r.data.get(), level->data.get(), focusx, focusy);
				return lox::make_nil();
			}
		)
		.add_function(
			"clear",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto color = ah.require_native<Rgb>();
				ah.complete();
				if (color == nullptr)
				{
					return nullptr;
				}

				auto data = r.data;
				if (data == nullptr)
				{
					lox::raise_error("must be called inside State.render()");
					return nullptr;
				}
				if (data->layer.has_value() == false)
				{
					lox::raise_error("need to setup virtual render area first");
					return nullptr;
				}
				render::RenderLayer2& layer = *data->layer;

				layer.batch->quadf(
					{},
					layer.viewport_aabb_in_worldspace.translate(data->focus),
					{},
					false,
					{color->r, color->g, color->b, 1.0f}
				);
				return lox::make_nil();
			}
		)
		.add_function(
			"rect",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());

				ah.complete();
				if (color == nullptr)
				{
					return nullptr;
				}

				if (width <= 0.0f)
				{
					lox::raise_error("width must be positive");
				}
				if (height <= 0.0f)
				{
					lox::raise_error("height must be positive");
				}

				auto data = r.data;
				if (data == nullptr)
				{
					lox::raise_error("must be called inside State.render()");
					return nullptr;
				}
				if (data->layer.has_value() == false)
				{
					lox::raise_error("need to setup virtual render area first");
					return nullptr;
				}

				render::RenderLayer2& layer = *data->layer;
				layer.batch->quadf(
					{},
					Rect{width, height}.translate(x, y),
					{},
					false,
					{color->r, color->g, color->b, 1.0f}
				);
				return lox::make_nil();
			}
		)
		.add_function(
			"text",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto font = ah.require_native<ScriptFont>();
				const auto height = static_cast<float>(ah.require_float());
				// auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				//const auto text = ah.require_string();
				const auto script_commands = ah.require_array();

				ah.complete();
				if (font == nullptr)
				{
					return nullptr;
				}

				auto data = r.data;
				if (data == nullptr)
				{
					lox::raise_error("must be called inside State.render()");
					return nullptr;
				}
				if (data->layer.has_value() == false)
				{
					lox::raise_error("need to setup virtual render area first");
					return nullptr;
				}

				render::RenderLayer2& layer = *data->layer;

				std::vector<render::TextCommand> commands;
				for (auto& obj: script_commands->values)
				{
					if (auto color = lox::as_native<Rgb>(obj); color)
					{
						commands.emplace_back(glm::vec4{color->r, color->g, color->b, 1.0f});
					}
					else
					{
						const auto str = obj->to_flat_string(lox::ToStringOptions::for_print());
						commands.emplace_back(str);
					}
				}
				font->font->print(layer.batch, height, x, y, commands);

				return lox::make_nil();
			}
		)
		.add_function(
			"sprite",
			[this](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto texture = ah.require_native<ScriptSprite>();
				// auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				const auto flip_x = ah.require_bool();
				/* const auto flip_y =*/ah.require_bool();
				ah.complete();

				if (texture == nullptr)
				{
					return nullptr;
				}

				auto data = r.data;
				if (data == nullptr)
				{
					lox::raise_error("must be called inside State.render()");
					return nullptr;
				}
				if (data->layer.has_value() == false)
				{
					lox::raise_error("need to setup virtual render area first");
					return nullptr;
				}

				render::RenderLayer2& layer = *data->layer;
				animations.emplace_back(texture->animation);
				const auto animation_index
					= static_cast<std::size_t>(std::max(0, texture->animation->current_index));
				Sprite& sprite = texture->sprites[animation_index];

				// void quad(std::optional<Texture*> texture, const Rectf& scr, const Recti& texturecoord, const glm::vec4& tint = glm::vec4(1.0f));
				const auto tint = glm::vec4(1.0f);
				const auto screen = Rectf{sprite.screen}.translate(x, y);
				layer.batch->quadf(sprite.texture.get(), screen, sprite.uv, flip_x, tint);

				return lox::make_nil();
			}
		);
	fyro->define_native_class<ScriptFont>("Font");
	fyro->define_native_function(
		"load_font",
		[this](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto path = ah.require_string();
			const auto size = ah.require_int();
			ah.complete();
			ScriptFont r;
			r.setup(path, static_cast<float>(size));
			loaded_fonts.emplace_back(r.font);
			return lox.make_native(r);
		}
	);
	fyro->define_native_class<ScriptSprite>("Sprite")
		.add_function(
			"set_size",
			[](ScriptSprite& t, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());
				ah.complete();
				for (auto& s: t.sprites)
				{
					s.screen = Rectf{width, height}.set_bottom_left(s.screen.left, s.screen.bottom);
				}
				return lox::make_nil();
			}
		)
		.add_function(
			"align",
			[](ScriptSprite& t, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				ah.complete();
				for (auto& s: t.sprites)
				{
					s.screen = s.screen.set_bottom_left(
						-s.screen.get_width() * x, -s.screen.get_height() * y
					);
				}
				return lox::make_nil();
			}
		);
	fyro->define_native_function(
		"load_image",
		[this](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto path = ah.require_string();
			ah.complete();
			ScriptSprite r;
			Sprite s;
			s.texture = texture_cache.get(path);
			s.screen = Rectf{
				static_cast<float>(s.texture->width), static_cast<float>(s.texture->height)
			};
			r.sprites.emplace_back(s);
			return lox.make_native(r);
		}
	);
	fyro->define_native_function(
		"sync_sprite_animations",
		[](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			auto to_sprite_array
				= [](std::shared_ptr<lox::Array> src) -> std::vector<lox::NativeRef<ScriptSprite>>
			{
				if (src == nullptr)
				{
					return {};
				}
				std::vector<lox::NativeRef<ScriptSprite>> dst;
				for (auto& v: src->values)
				{
					if (auto native = lox::as_native<ScriptSprite>(v); native)
					{
						dst.emplace_back(native);
					}
					else
					{
						// todo(Gustav): expand lox so this is part of the argument handler eval
						// todo(Gustav): add argument index here for better error handling
						lox::raise_error("element in array is not a Sprite");
					}
				}
				return dst;
			};
			auto sprite_array = to_sprite_array(ah.require_array());
			ah.complete();

			std::shared_ptr<SpriteAnimation> main_animation;
			for (auto& sp: sprite_array)
			{
				if (main_animation == nullptr)
				{
					main_animation = sp->animation;
				}
				else
				{
					// todo(Gustav): verify that animation has same number of frames!
					if (sp->animation->total_sprites != main_animation->total_sprites)
					{
						lox::raise_error(
							"unable to sync: sprite animations have different number of frames"
						);
					}
					sp->animation = main_animation;
				}
			}

			return lox::make_nil();
		}
	);
	fyro->define_native_function(
		"load_sprite",
		[this](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			auto to_vec2i_array = [](std::shared_ptr<lox::Array> src) -> std::vector<glm::ivec2>
			{
				if (src == nullptr)
				{
					return {};
				}
				std::vector<glm::ivec2> dst;
				for (auto& v: src->values)
				{
					if (auto native = lox::as_native<glm::ivec2>(v); native)
					{
						dst.emplace_back(*native);
					}
					else
					{
						// todo(Gustav): expand lox so this is part of the argument handler eval
						// todo(Gustav): add argument index here for better error handling
						lox::raise_error("element in array is not a vec2i");
					}
				}
				return dst;
			};
			const auto path = ah.require_string();
			const auto tiles_per_x = static_cast<int>(ah.require_int());
			const auto tiles_per_y = static_cast<int>(ah.require_int());
			const auto anim_speed = static_cast<float>(ah.require_float());
			auto tiles_array = to_vec2i_array(ah.require_array());
			ah.complete();

			if (tiles_array.empty())
			{
				// lox::raise_error("sprite array was empty");

				for (int y = 0; y < tiles_per_y; y += 1)
				{
					for (int x = 0; x < tiles_per_x; x += 1)
					{
						tiles_array.emplace_back(x, y);
					}
				}
			}

			ScriptSprite r;
			r.animation->setup(anim_speed, static_cast<int>(tiles_array.size()));
			for (const auto& tile: tiles_array)
			{
				Sprite s;
				s.texture = texture_cache.get(path);
				const auto iw = static_cast<float>(s.texture->width);
				const auto ih = static_cast<float>(s.texture->height);
				s.screen = Rectf{iw, ih};

				const auto tile_pix_w = iw / static_cast<float>(tiles_per_x);
				const auto tile_pix_h = ih / static_cast<float>(tiles_per_y);
				const auto tile_frac_w = tile_pix_w / iw;
				const auto tile_frac_h = tile_pix_h / ih;
				const auto dx = tile_frac_w * static_cast<float>(tile.x);
				const auto dy = tile_frac_h * static_cast<float>(tile.y);
				s.uv = Rectf{tile_frac_w, tile_frac_h}.translate(dx, dy);
				r.sprites.emplace_back(s);
			}
			return lox.make_native(r);
		}
	);
	fyro->define_native_class<ScriptPlayer>("Player")
		.add_native_getter<InputFrame>(
			"current",
			[this](const ScriptPlayer& s) { return lox.make_native(s.player->current_frame); }
		)
		.add_native_getter<InputFrame>(
			"last", [this](const ScriptPlayer& s) { return lox.make_native(s.player->last_frame); }
		)
		.add_function(
			"run_haptics",
			[](ScriptPlayer& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				float force = static_cast<float>(ah.require_float());
				float life = static_cast<float>(ah.require_float());
				ah.complete();
				if (r.player == nullptr)
				{
					lox::raise_error("Player not created from input!");
				}
				r.player->run_haptics(force, life);
				return lox::make_nil();
			}
		);
	fyro->define_native_class<ScriptRandom>("Random").add_function(
		"next_int",
		[](ScriptRandom& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto max_value = ah.require_int();
			ah.complete();
			const auto value = r.next_int(max_value);
			return lox::make_number_int(value);
		}
	);
	fyro->define_native_class<InputFrame>("InputFrame")
		.add_getter<lox::Tf>(
			"axis_left_x", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_left_x); }
		)
		.add_getter<lox::Tf>(
			"axis_left_y", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_left_y); }
		)
		.add_getter<lox::Tf>(
			"axis_right_x", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_right_x); }
		)
		.add_getter<lox::Tf>(
			"axis_right_y", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_right_y); }
		)
		.add_getter<lox::Tf>(
			"axis_trigger_left",
			[](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_trigger_left); }
		)
		.add_getter<lox::Tf>(
			"axis_trigger_right",
			[](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_trigger_right); }
		)
		.add_getter<bool>("button_a", [](const InputFrame& f) { return f.button_a; })
		.add_getter<bool>("button_b", [](const InputFrame& f) { return f.button_b; })
		.add_getter<bool>("button_x", [](const InputFrame& f) { return f.button_x; })
		.add_getter<bool>("button_y", [](const InputFrame& f) { return f.button_y; })
		.add_getter<bool>("button_back", [](const InputFrame& f) { return f.button_back; })
		.add_getter<bool>("button_guide", [](const InputFrame& f) { return f.button_guide; })
		.add_getter<bool>("button_start", [](const InputFrame& f) { return f.button_start; })
		.add_getter<bool>(
			"button_leftstick", [](const InputFrame& f) { return f.button_leftstick; }
		)
		.add_getter<bool>(
			"button_rightstick", [](const InputFrame& f) { return f.button_rightstick; }
		)
		.add_getter<bool>(
			"button_leftshoulder", [](const InputFrame& f) { return f.button_leftshoulder; }
		)
		.add_getter<bool>(
			"button_rightshoulder", [](const InputFrame& f) { return f.button_rightshoulder; }
		)
		.add_getter<bool>("button_dpad_up", [](const InputFrame& f) { return f.button_dpad_up; })
		.add_getter<bool>(
			"button_dpad_down", [](const InputFrame& f) { return f.button_dpad_down; }
		)
		.add_getter<bool>(
			"button_dpad_left", [](const InputFrame& f) { return f.button_dpad_left; }
		)
		.add_getter<bool>(
			"button_dpad_right", [](const InputFrame& f) { return f.button_dpad_right; }
		)
		.add_getter<bool>("button_misc1", [](const InputFrame& f) { return f.button_misc1; })
		.add_getter<bool>("button_paddle1", [](const InputFrame& f) { return f.button_paddle1; })
		.add_getter<bool>("button_paddle2", [](const InputFrame& f) { return f.button_paddle2; })
		.add_getter<bool>("button_paddle3", [](const InputFrame& f) { return f.button_paddle3; })
		.add_getter<bool>("button_paddle4", [](const InputFrame& f) { return f.button_paddle4; })
		.add_getter<bool>("button_touchpad", [](const InputFrame& f) { return f.button_touchpad; });
}

void ExampleGame::on_update(float dt)
{
	input.update(dt);
	input.start_new_frame();

	for (auto& anim: animations)
	{
		anim->update(dt);
	}

	if (state)
	{
		state->update(dt);
	}
	if (next_state != nullptr)
	{
		state = std::move(next_state);
	}
}

void ExampleGame::on_render(const render::RenderCommand& rc)
{
	animations.clear();
	if (state)
	{
		state->render(rc);
	}
	else
	{
		// todo(Gustav): draw some basic thing here since to state is set?
		auto r = render::with_layer2(
			rc, render::LayoutData{render::ViewportStyle::extended, 200.0f, 200.0f}
		);
		r.batch->quadf({}, r.viewport_aabb_in_worldspace, {}, false, {0.8, 0.8, 0.8, 1.0f});
		r.batch->submit();
	}
}

void ExampleGame::on_mouse_position(const render::InputCommand&, const glm::ivec2&)
{
}

void ExampleGame::on_added_controller(SDL_GameController* controller)
{
	input.add_controller(controller);
}

void ExampleGame::on_lost_joystick_instance(int instance_id)
{
	input.lost_controller(instance_id);
}
