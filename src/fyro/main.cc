#include <iostream>
#include <fstream>
#include <filesystem>
#include <optional>
#include <random>

#include <nlohmann/json.hpp>
#include "physfs.h"

#include "lox/lox.h"
#include "lox/printhandler.h"

#include "fyro/fyro.h"
#include "fyro/render/font.h"
#include "fyro/render/texture.h"


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

std::vector<std::string> physfs_files_in_dir(const std::string& dir)
{
	constexpr const auto callback = [](void *data, const char *origdir, const char *fname) -> PHYSFS_EnumerateCallbackResult
	{
		auto* ret = static_cast<std::vector<std::string>*>(data);
		ret->emplace_back(std::string(origdir) + fname);
		return PHYSFS_ENUM_OK;
	};

	std::vector<std::string> r;
	auto ok = PHYSFS_enumerate(dir.c_str(), callback, &r);

	if(ok == 0)
	{
		std::cerr << "failed to enumerate " << dir << "\n";
	}

	return r;
}

std::vector<char> read_file_to_bytes(const std::string& path)
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
		constexpr u64 buffer_size = 1024;
		char buffer[buffer_size] = {0,};
		const auto read = PHYSFS_readBytes(file, buffer, buffer_size);
		if(read > 0)
		{
			ret.insert(ret.end(), buffer, buffer+read);
		}
	}

	ret.emplace_back('\0');

	PHYSFS_close(file);
	return ret;
}


std::string read_file_to_string(const std::string& path)
{
	auto ret = read_file_to_bytes(path);
	ret.emplace_back('\0');
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
		x.append("while reading " + path);
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

struct KeyboardMapping
{
	std::optional<SDL_Scancode> axis_left_x_neg; std::optional<SDL_Scancode> axis_left_x_pos;
	std::optional<SDL_Scancode> axis_left_y_neg; std::optional<SDL_Scancode> axis_left_y_pos;
	std::optional<SDL_Scancode> axis_right_x_neg; std::optional<SDL_Scancode> axis_right_x_pos;
	std::optional<SDL_Scancode> axis_right_y_neg; std::optional<SDL_Scancode> axis_right_y_pos;

	std::optional<SDL_Scancode> axis_trigger_left;
	std::optional<SDL_Scancode> axis_trigger_right;

	std::optional<SDL_Scancode> button_a;
	std::optional<SDL_Scancode> button_b;
	std::optional<SDL_Scancode> button_x;
	std::optional<SDL_Scancode> button_y;
	std::optional<SDL_Scancode> button_back;
	std::optional<SDL_Scancode> button_guide;
	std::optional<SDL_Scancode> button_start;
	std::optional<SDL_Scancode> button_leftstick;
	std::optional<SDL_Scancode> button_rightstick;
	std::optional<SDL_Scancode> button_leftshoulder;
	std::optional<SDL_Scancode> button_rightshoulder;
	std::optional<SDL_Scancode> button_dpad_up;
	std::optional<SDL_Scancode> button_dpad_down;
	std::optional<SDL_Scancode> button_dpad_left;
	std::optional<SDL_Scancode> button_dpad_right;
	std::optional<SDL_Scancode> button_misc1;
	std::optional<SDL_Scancode> button_paddle1;
	std::optional<SDL_Scancode> button_paddle2;
	std::optional<SDL_Scancode> button_paddle3;
	std::optional<SDL_Scancode> button_paddle4;
	std::optional<SDL_Scancode> button_touchpad;
};

KeyboardMapping create_default_mapping_for_player1()
{
	KeyboardMapping m;

	m.axis_left_x_neg = SDL_SCANCODE_LEFT;
	m.axis_left_x_pos = SDL_SCANCODE_RIGHT;

	m.axis_left_y_neg = SDL_SCANCODE_DOWN;
	m.axis_left_y_pos = SDL_SCANCODE_UP;

	m.button_a = SDL_SCANCODE_S;
	m.button_b = SDL_SCANCODE_D;
	m.button_x = SDL_SCANCODE_X;
	m.button_y = SDL_SCANCODE_C;

	m.button_start = SDL_SCANCODE_RETURN;

	return m;
}

struct InputFrame
{
	float axis_left_x = 0.0f;
	float axis_left_y = 0.0f;
	float axis_right_x = 0.0f;
	float axis_right_y = 0.0f;

	float axis_trigger_left = 0.0f;
	float axis_trigger_right = 0.0f;

	bool button_a = false;
	bool button_b = false;
	bool button_x = false;
	bool button_y = false;
	bool button_back = false;
	bool button_guide = false;
	bool button_start = false;
	bool button_leftstick = false;
	bool button_rightstick = false;
	bool button_leftshoulder = false;
	bool button_rightshoulder = false;
	bool button_dpad_up = false;
	bool button_dpad_down = false;
	bool button_dpad_left = false;
	bool button_dpad_right = false;
	bool button_misc1 = false;
	bool button_paddle1 = false;
	bool button_paddle2 = false;
	bool button_paddle3 = false;
	bool button_paddle4 = false;
	bool button_touchpad = false;
};

InputFrame capture_keyboard(const KeyboardMapping& mapping)
{
	int numkeys = 0;
	const Uint8* keys = SDL_GetKeyboardState(&numkeys);

	const auto get = [keys, numkeys](std::optional<SDL_Scancode> sc) -> bool
	{
		if(sc)
		{
			if(*sc >= numkeys) { return false; }
			if(*sc < 0) { return false; }
			return keys[*sc] == 1;
		}
		else
		{
			return false;
		}
	};

	const float axis_left_x_neg = get(mapping.axis_left_x_neg) ? -1.0f : 0.0f;
	const float axis_left_x_pos = get(mapping.axis_left_x_pos) ?  1.0f : 0.0f;
	const float axis_left_y_neg = get(mapping.axis_left_y_neg) ? -1.0f : 0.0f;
	const float axis_left_y_pos = get(mapping.axis_left_y_pos) ?  1.0f : 0.0f;

	const float axis_right_x_neg = get(mapping.axis_right_x_neg) ? -1.0f : 0.0f;
	const float axis_right_x_pos = get(mapping.axis_right_x_pos) ?  1.0f : 0.0f;
	const float axis_right_y_neg = get(mapping.axis_right_y_neg) ? -1.0f : 0.0f;
	const float axis_right_y_pos = get(mapping.axis_right_y_pos) ?  1.0f : 0.0f;

	InputFrame r;

	r.axis_left_x = axis_left_x_neg + axis_left_x_pos;
	r.axis_left_y = axis_left_y_neg + axis_left_y_pos;
	r.axis_right_x = axis_right_x_neg + axis_right_x_pos;
	r.axis_right_y = axis_right_y_neg + axis_right_y_pos;

	r.axis_trigger_left = get(mapping.axis_trigger_left) ? 1.0f : 0.0f;
	r.axis_trigger_right = get(mapping.axis_trigger_right) ? 1.0f : 0.0f;

	r.button_a = get(mapping.button_a);
	r.button_b = get(mapping.button_b);
	r.button_x = get(mapping.button_x);
	r.button_y = get(mapping.button_y);
	r.button_back = get(mapping.button_back);
	r.button_guide = get(mapping.button_guide);
	r.button_start = get(mapping.button_start);
	r.button_leftstick = get(mapping.button_leftstick);
	r.button_rightstick = get(mapping.button_rightstick);
	r.button_leftshoulder = get(mapping.button_leftshoulder);
	r.button_rightshoulder = get(mapping.button_rightshoulder);
	r.button_dpad_up = get(mapping.button_dpad_up);
	r.button_dpad_down = get(mapping.button_dpad_down);
	r.button_dpad_left = get(mapping.button_dpad_left);
	r.button_dpad_right = get(mapping.button_dpad_right);
	r.button_misc1 = get(mapping.button_misc1);
	r.button_paddle1 = get(mapping.button_paddle1);
	r.button_paddle2 = get(mapping.button_paddle2);
	r.button_paddle3 = get(mapping.button_paddle3);
	r.button_paddle4 = get(mapping.button_paddle4);
	r.button_touchpad = get(mapping.button_touchpad);

	return r;
}

float make_stable(float f)
{
	const float ff = f > 0.0f ? f : -f;

	if (ff <= 0.18f) { return 0.0f; }
	else { return f; }
}

InputFrame capture_gamecontroller(SDL_GameController* gamecontroller)
{
	// The state is a value ranging from -32768 to 32767.
	const auto from_axis = [gamecontroller](SDL_GameControllerAxis axis) -> float
	{
		const auto s = SDL_GameControllerGetAxis(gamecontroller, axis);
		if(s >= 0)
		{
			return make_stable(static_cast<float>(s)/32767.0f);
		}
		else
		{
			// don't include sign here as s is already negative
			return make_stable(static_cast<float>(s)/32768.0f);
		}
	};

	// Triggers, however, range from 0 to 32767 (they never return a negative value).
	const auto from_trigger = [gamecontroller](SDL_GameControllerAxis axis) -> float
	{
		const auto s = SDL_GameControllerGetAxis(gamecontroller, axis);
		union { Sint16 s; Uint16 u;} cast;
		cast.s = s;
		return make_stable(static_cast<float>(cast.u) / 32767.0f);
	};

	const auto from_button = [gamecontroller](SDL_GameControllerButton button) -> bool
	{
		return SDL_GameControllerGetButton(gamecontroller, button) == 1;
	};

	InputFrame r;

	r.axis_left_x = from_axis(SDL_CONTROLLER_AXIS_LEFTX);
	r.axis_left_y = -from_axis(SDL_CONTROLLER_AXIS_LEFTY);
	r.axis_right_x = from_axis(SDL_CONTROLLER_AXIS_RIGHTX);
	r.axis_right_y = -from_axis(SDL_CONTROLLER_AXIS_RIGHTY);

	r.axis_trigger_left = from_trigger(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
	r.axis_trigger_right = from_trigger(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

	r.button_a = from_button(SDL_CONTROLLER_BUTTON_A);
	r.button_b = from_button(SDL_CONTROLLER_BUTTON_B);
	r.button_x = from_button(SDL_CONTROLLER_BUTTON_X);
	r.button_y = from_button(SDL_CONTROLLER_BUTTON_Y);
	r.button_back = from_button(SDL_CONTROLLER_BUTTON_BACK);
	r.button_guide = from_button(SDL_CONTROLLER_BUTTON_GUIDE);
	r.button_start = from_button(SDL_CONTROLLER_BUTTON_START);
	r.button_leftstick = from_button(SDL_CONTROLLER_BUTTON_LEFTSTICK);
	r.button_rightstick = from_button(SDL_CONTROLLER_BUTTON_RIGHTSTICK);
	r.button_leftshoulder = from_button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
	r.button_rightshoulder = from_button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
	r.button_dpad_up = from_button(SDL_CONTROLLER_BUTTON_DPAD_UP);
	r.button_dpad_down = from_button(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	r.button_dpad_left = from_button(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	r.button_dpad_right = from_button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
	
	#if SDL_VERSION_ATLEAST(2, 0, 14)
	r.button_misc1 = from_button(SDL_CONTROLLER_BUTTON_MISC1);
	r.button_paddle1 = from_button(SDL_CONTROLLER_BUTTON_PADDLE1);
	r.button_paddle2 = from_button(SDL_CONTROLLER_BUTTON_PADDLE2);
	r.button_paddle3 = from_button(SDL_CONTROLLER_BUTTON_PADDLE3);
	r.button_paddle4 = from_button(SDL_CONTROLLER_BUTTON_PADDLE4);
	r.button_touchpad = from_button(SDL_CONTROLLER_BUTTON_TOUCHPAD);
	#endif

	return r;
}

struct HapticsEffect
{
	float force;
	float life;

	HapticsEffect(float f, float l)\
		: force(f)
		, life(l)
	{
	}

	bool is_alive() const
	{
		return life > 0.0f;
	}

	void update(float dt)
	{
		life -= dt;
	}
};



struct HapticsEngine
{
	SDL_Haptic* haptic;
	std::vector<HapticsEffect> effects;

	explicit HapticsEngine(SDL_Joystick* joystick) : haptic(create_haptic(joystick))
	{
	}

	void on_imgui()
	{
		for (std::size_t index =0; index < effects.size(); index += 1)
		{
			const auto text = "{}"_format(effects[index].life);
			ImGui::TextUnformatted(text.c_str());
		}
	}

	void run(float force, float life)
	{
		if (haptic == nullptr) { return; }
		auto e = HapticsEffect{ force, life };
		effects.emplace_back(std::move(e));
	}

	~HapticsEngine()
	{
		if (haptic != nullptr)
		{
			SDL_HapticClose(haptic);
		}
	}

	void update(float dt)
	{
		for (auto& e : effects)
		{
			e.update(dt);
		}

		effects.erase(std::remove_if(effects.begin(),
			effects.end(),
			[&](const HapticsEffect& e)-> bool
			{ return e.is_alive()  == false; }),
			effects.end());
		
		enable_disable_rumble();
	}

	void enable_disable_rumble()
	{
		if (haptic == nullptr) { return; }

		const auto current = get_rumble_effect();

		auto sdl_stop_rumble = [this]()
		{
			SDL_HapticRumbleStop(haptic);
		};
		auto sdl_start_rumble = [this, &current]()
		{
			SDL_HapticRumblePlay(haptic, *current, SDL_HAPTIC_INFINITY);
		};

		if (current && last_rumble)
		{
			// update if different
			if (*current != *last_rumble)
			{
				sdl_stop_rumble();
				sdl_start_rumble();
			}
		}
		else if (current)
		{
			assert(!last_rumble);
			sdl_start_rumble();
		}
		else if(last_rumble)
		{
			assert(!current);
			sdl_stop_rumble();
		}
		else
		{
			// no rumble action needed
		}

		last_rumble = current;
	}

	std::optional<float> last_rumble = std::nullopt;
	
	std::optional<float> get_rumble_effect() const
	{
		std::optional<float> force = std::nullopt;
		for (auto& e : effects)
		{
			if (force)
			{
				force = *force + e.force;
			}
			else
			{
				force = e.force;
			}
		}

		if (force)
		{
			return std::min(1.0f, *force);
		}
		else
		{
			return std::nullopt;
		}
	}

	HapticsEngine(const HapticsEngine&) = delete;
	void operator=(const HapticsEngine&) = delete;
	HapticsEngine(HapticsEngine&&) = delete;
	void operator=(HapticsEngine&&) = delete;

	static SDL_Haptic* create_haptic(SDL_Joystick* joystick)
	{
		SDL_Haptic* haptic;

		// Open the device
		haptic = SDL_HapticOpenFromJoystick(joystick);
		if (haptic == NULL)
		{
			// Most likely joystick isn't haptic
			return nullptr;
		}

		const auto initialized = SDL_HapticRumbleInit(haptic);
		if (initialized != 0)
		{
			std::cout << "SDL: rumble not supported: " << SDL_GetError() << "\n";
			SDL_HapticClose(haptic);
			return nullptr;
		}

		return haptic;
	}
};


struct InputDevice
{
	InputDevice() = default;
	virtual ~InputDevice() = default;

	bool free = true;

	virtual std::string get_name() = 0;
	virtual bool is_connected() = 0;
	virtual InputFrame capture_frame() = 0;
	virtual void run_haptics(float force, float life) = 0;
	virtual void update(float dt) = 0;
	virtual void on_imgui() = 0;
};

struct GlobalMappings
{
	std::vector<KeyboardMapping> mappings;
};

struct InputDevice_Keyboard : InputDevice
{
	GlobalMappings* mappings = nullptr;
	std::size_t index = 0;

	InputDevice_Keyboard(GlobalMappings* m, std::size_t i)
		: mappings(m)
		, index(i)
	{
	}

	std::string get_name() override
	{
		// todo(Gustav): use format to print index
		return "keyboard";
	}

	bool is_connected() override
	{
		return true;
	}

	InputFrame capture_frame() override
	{
		const auto r = capture_keyboard(mappings->mappings[index]);
		return r;
	}

	void run_haptics(float, float) override
	{
	}

	void update(float) override
	{
	}

	void on_imgui() override
	{
	}
};

struct InputDevice_Gamecontroller : InputDevice
{
	SDL_GameController* controller;
	HapticsEngine haptics;
	std::string name;

	explicit InputDevice_Gamecontroller(SDL_GameController* c)
		: controller(c)
		, haptics(SDL_GameControllerGetJoystick(c))
		, name(collect_name(c))
	{
	}

	static std::string collect_name(SDL_GameController* controller)
	{
		if (controller == nullptr) { return "missing controller"; }

		const char* name = SDL_GameControllerName(controller);
		if (name) { return name; }
		else { return "<unnamed controller>"; }
	}

	~InputDevice_Gamecontroller()
	{
		clear_controller();
	}

	void clear_controller()
	{
		if (controller)
		{
			SDL_GameControllerClose(controller);
			controller = nullptr;
		}
	}

	int get_device_index()
	{
		auto* joystick = SDL_GameControllerGetJoystick(controller);
		return SDL_JoystickInstanceID(joystick);
	}

	std::string get_name() override
	{
		return name;
	}

	bool is_connected() override
	{
		return controller != nullptr;
	}

	InputFrame capture_frame() override
	{
		if (controller)
		{
			return capture_gamecontroller(controller);
		}
		else
		{
			return InputFrame{};
		}
	}

	void run_haptics(float force, float life) override
	{
		haptics.run(force, life);
	}

	void update(float dt) override
	{
		haptics.update(dt);
	}

	void on_imgui() override
	{
		ImGui::TextUnformatted(name.c_str());
		haptics.on_imgui();
	}
};


struct Player
{
	int age = 0;
	InputFrame current_frame;
	InputFrame last_frame;

	std::shared_ptr<InputDevice> device;

	Player() = default;

	~Player()
	{
		if(device)
		{
			device->free = true;
		}
	}

	void on_imgui()
	{
		if (device)
		{
			ImGui::Begin("player");
			device->on_imgui();
			ImGui::End();
		}
	}

	bool is_connected()
	{
		return device && device->is_connected();
	}

	void update_frame()
	{
		last_frame = current_frame;
		current_frame =  device
			? device->capture_frame()
			: InputFrame{}
			;
	}

	void run_haptics(float force, float life)
	{
		if (device)
		{
			device->run_haptics(force, life);
		}
	}

	void update(float dt)
	{
		if (device)
		{
			device->update(dt);
		}
	}
};

struct Input
{
	std::map<int, std::shared_ptr<InputDevice_Gamecontroller>> controllers;
	std::vector<std::shared_ptr<InputDevice_Keyboard>> keyboards;
	std::vector<std::shared_ptr<Player>> players;
	std::size_t next_player = 0;

	void add_controller(SDL_GameController* controller)
	{
		auto ctrl = std::make_shared<InputDevice_Gamecontroller>(controller);
		const auto index = ctrl->get_device_index();
		std::cout << "Connected " << index << ": " << ctrl->get_name() << "\n";
		controllers.insert({index, ctrl});
	}

	void lost_controller(int instance_id)
	{
		if (auto found = controllers.find(instance_id); found != controllers.end())
		{
			std::cout << "Lost " << found->first << ": " << found->second->get_name() << "\n";
			found->second->clear_controller();
			controllers.erase(found);
		}
	}

	void on_imgui()
	{
		for (auto& p : players)
		{
			p->on_imgui();
		}
	}

	void add_keyboard(std::shared_ptr<InputDevice_Keyboard> kb)
	{
		keyboards.emplace_back(kb);
	}

	// remove lost controller

	std::shared_ptr<InputDevice> find_device()
	{
		for(auto& co: controllers)
		{
			auto& c = co.second;
			if(c->free && c->capture_frame().button_start)
			{
				c->free = false;
				return c;
			}
		}

		for(auto& c: keyboards)
		{
			if(c->free && c->capture_frame().button_start)
			{
				c->free = false;
				return c;
			}
		}

		return nullptr;
	}

	void start_new_frame()
	{
		next_player = 0;

		// todo(Gustav): remove old players

		// age players
		for(auto& p: players) { p->age += 1; }
	}

	std::shared_ptr<Player> capture_player()
	{
		if(next_player >= players.size())
		{
			next_player += 1;
			auto r = std::make_shared<Player>();
			r->device = find_device();
			players.emplace_back(r);
			r->update_frame();
			return r;
		}
		else
		{
			auto r = players[next_player];
			next_player += 1;
			if(r->is_connected() == false) { r->device = find_device(); }
			r->update_frame();
			return r;
		}
	}

	void update(float dt)
	{
		for (auto& p : players)
		{
			p->update(dt);
		}
	}
};

struct ScriptPlayer
{
	std::shared_ptr<Player> player;
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
		: generator( create_engine() )
	{
	}

	lox::Ti next_int(lox::Ti max)
	{
		std::uniform_int_distribution<lox::Ti> distribution(0, max-1);
		return distribution(generator);
	}
};


struct ScriptFont
{
	std::shared_ptr<render::Font> font;

	void setup(const std::string& path, float height)
	{
		auto bytes = read_file_to_bytes(path);
		font = std::make_shared<render::Font>(reinterpret_cast<const unsigned char*>(bytes.data()), height);
	}
};

std::shared_ptr<render::Texture>
load_texture(const std::string& path)
{
	const auto bytes = read_file_to_bytes(path);
	return std::make_shared<render::Texture>
	(
		render::load_image_from_bytes
		(
			reinterpret_cast<const unsigned char*>(bytes.data()),
			static_cast<int>(bytes.size()),
			render::TextureEdge::repeat,
			render::TextureRenderStyle::pixel,
			render::Transparency::include
		)
	);
}

struct Sprite
{
	Sprite() : screen(1.0f, 1.0f)
	{
	}
	std::shared_ptr<render::Texture> texture;
	Rectf screen;
};

struct ScriptSprite
{
	std::vector<Sprite> sprites;
};

template<typename TSource, typename TData>
struct Cache
{
	using Loader = std::function<std::shared_ptr<TData>(const TSource&)>;
	
	Loader load;
	std::unordered_map<TSource, std::weak_ptr<TData>> loaded;

	explicit Cache(Loader&& l) : load(l) {}

	std::shared_ptr<TData> get(const TSource& source)
	{
		if(auto found = loaded.find(source); found != loaded.end())
		{
			auto ret = found->second.lock();
			if(ret != nullptr)
			{
				return ret;
			}
		}

		auto ret = load(source);
		assert(ret != nullptr);
		loaded[source] = ret;
		return ret;
	}
};

struct ExampleGame : public Game
{
	lox::Lox lox;

	GlobalMappings keyboards;
	Input input;

	std::unique_ptr<State> next_state;
	std::unique_ptr<State> state;

	std::vector<std::shared_ptr<render::Font>> loaded_fonts; // todo(Gustav): replace with cache
	Cache<std::string, render::Texture> texture_cache;

	ExampleGame()
		: lox(std::make_unique<PrintLoxError>(), [](const std::string& str)
		{
			std::cout << str << "\n";
		})
		, texture_cache([](const std::string& path)
		{
			return load_texture(path);
		})
	{
		// todo(Gustav): read/write to json and provide ui for adding new mappings
		keyboards.mappings.emplace_back(create_default_mapping_for_player1());
		input.add_keyboard( std::make_shared<InputDevice_Keyboard>(&keyboards, 0) );
		bind_functions();
	}

	void on_imgui() override
	{
		for(auto& f: loaded_fonts)
		{
			f->imgui();
		}
		input.on_imgui();
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

		fyro->define_native_function("get_input", [this](lox::Callable*, lox::ArgumentHelper& arguments)
		{
			arguments.complete();
			auto frame = input.capture_player();
			return lox.make_native<ScriptPlayer>(ScriptPlayer{frame});
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
			.add_function("windowbox", [](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());
				ah.complete();

				if(width <= 0.0f) { lox::raise_error("width must be positive"); }
				if(height <= 0.0f) { lox::raise_error("height must be positive"); }
				if(r.data == nullptr) { lox::raise_error("must be called inside State.render()"); return nullptr; }

				r.data->layer = render::with_layer2(r.data->rc, render::LayoutData{render::ViewportStyle::black_bars, width, height});
				return lox::make_nil();
			})
			.add_function("clear", [](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto color = ah.require_native<Rgb>();
				ah.complete();
				if(color == nullptr) { return nullptr;}

				auto data = r.data;
				if(data == nullptr) { lox::raise_error("must be called inside State.render()"); return nullptr; }
				if(data->layer.has_value() == false) { lox::raise_error("need to setup virtual render area first"); return nullptr; }
				
				render::RenderLayer2& layer = *data->layer;
				layer.batch->quad({}, layer.viewport_aabb_in_worldspace, {}, {color->r, color->g, color->b, 1.0f});
				return lox::make_nil();
			})
			.add_function("rect", [](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());

				ah.complete();
				if(color == nullptr) { return nullptr; }

				if(width <= 0.0f) { lox::raise_error("width must be positive"); }
				if(height <= 0.0f) { lox::raise_error("height must be positive"); }

				auto data = r.data;
				if(data == nullptr) { lox::raise_error("must be called inside State.render()"); return nullptr; }
				if(data->layer.has_value() == false) { lox::raise_error("need to setup virtual render area first"); return nullptr; }
				
				render::RenderLayer2& layer = *data->layer;
				layer.batch->quad({}, Rect{width, height}.translate(x, y), {}, {color->r, color->g, color->b, 1.0f}
				);
				return lox::make_nil();
			})
			.add_function("text", [](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto font = ah.require_native<ScriptFont>();
				const auto height = static_cast<float>(ah.require_float());
				// auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				//const auto text = ah.require_string();
				const auto script_commands = ah.require_array();

				ah.complete();
				if(font == nullptr) { return nullptr; }

				auto data = r.data;
				if(data == nullptr) { lox::raise_error("must be called inside State.render()"); return nullptr; }
				if(data->layer.has_value() == false) { lox::raise_error("need to setup virtual render area first"); return nullptr; }
				
				render::RenderLayer2& layer = *data->layer;

				std::vector<render::TextCommand> commands;
				for(auto& obj: script_commands->values)
				{
					if(auto color = lox::as_native<Rgb>(obj); color)
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
			})
			.add_function("sprite", [](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto texture = ah.require_native<ScriptSprite>();
				// auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());

				ah.complete();
				if(texture == nullptr) { return nullptr; }

				auto data = r.data;
				if(data == nullptr) { lox::raise_error("must be called inside State.render()"); return nullptr; }
				if(data->layer.has_value() == false) { lox::raise_error("need to setup virtual render area first"); return nullptr; }
				
				render::RenderLayer2& layer = *data->layer;
				Sprite& sprite = texture->sprites[0]; // todo(Gustav): select current sprite better

				// void quad(std::optional<Texture*> texture, const Rectf& scr, const Recti& texturecoord, const glm::vec4& tint = glm::vec4(1.0f));
				const auto tint = glm::vec4(1.0f);
				const auto screen = Rectf{sprite.screen}.translate(x, y);
				layer.batch->quad(sprite.texture.get(), screen, Rectf{1.0f, 1.0f}, tint);
				
				return lox::make_nil();
			})
			;
		fyro->define_native_class<ScriptFont>("Font");
		fyro->define_native_function("load_font", [this](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto path = ah.require_string();
			const auto size = ah.require_int();
			ah.complete();
			ScriptFont r;
			r.setup(path, static_cast<float>(size));
			loaded_fonts.emplace_back(r.font);
			return lox.make_native(r);
		});
		fyro->define_native_class<ScriptSprite>("Sprite")
			.add_function("set_size", [](ScriptSprite& t, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto width = static_cast<float>(ah.require_float());
				const auto height = static_cast<float>(ah.require_float());
				ah.complete();
				for(auto& s: t.sprites)
				{
					s.screen = Rectf{width, height}.set_bottom_left(s.screen.left, s.screen.bottom);
				}
				return lox::make_nil();
			})
			.add_function("align", [](ScriptSprite& t, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				ah.complete();
				for(auto& s: t.sprites)
				{
					s.screen = s.screen.set_bottom_left( -s.screen.get_width() * x, -s.screen.get_height()*y );
				}
				return lox::make_nil();
			})
			;
		fyro->define_native_function("load_image", [this](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto path = ah.require_string();
			ah.complete();
			ScriptSprite r;
			Sprite s;
			s.texture = texture_cache.get(path);
			s.screen = Rectf{static_cast<float>(s.texture->width), static_cast<float>(s.texture->height)};
			r.sprites.emplace_back(s);
			return lox.make_native(r);
		});
		fyro->define_native_class<ScriptPlayer>("Player")
			.add_native_getter<InputFrame>("current", [this](const ScriptPlayer& s) { return lox.make_native(s.player->current_frame); })
			.add_native_getter<InputFrame>("last", [this](const ScriptPlayer& s) { return lox.make_native(s.player->last_frame); })
			.add_function("run_haptics", [](ScriptPlayer&r, lox::ArgumentHelper& ah)->std::shared_ptr<lox::Object>
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
			})
			;
		fyro->define_native_class<ScriptRandom>("Random")
			.add_function("next_int", [](ScriptRandom& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto max_value = ah.require_int();
				ah.complete();
				const auto value = r.next_int(max_value);
				return lox::make_number_int(value);
			});
		fyro->define_native_class<InputFrame>("InputFrame")
			.add_getter<lox::Tf>("axis_left_x", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_left_x); })
			.add_getter<lox::Tf>("axis_left_y", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_left_y); })
			.add_getter<lox::Tf>("axis_right_x", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_right_x); })
			.add_getter<lox::Tf>("axis_right_y", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_right_y); })
			.add_getter<lox::Tf>("axis_trigger_left", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_trigger_left); })
			.add_getter<lox::Tf>("axis_trigger_right", [](const InputFrame& f) { return static_cast<lox::Tf>(f.axis_trigger_right); })
			.add_getter<bool>("button_a", [](const InputFrame& f) { return f.button_a; })
			.add_getter<bool>("button_b", [](const InputFrame& f) { return f.button_b; })
			.add_getter<bool>("button_x", [](const InputFrame& f) { return f.button_x; })
			.add_getter<bool>("button_y", [](const InputFrame& f) { return f.button_y; })
			.add_getter<bool>("button_back", [](const InputFrame& f) { return f.button_back; })
			.add_getter<bool>("button_guide", [](const InputFrame& f) { return f.button_guide; })
			.add_getter<bool>("button_start", [](const InputFrame& f) { return f.button_start; })
			.add_getter<bool>("button_leftstick", [](const InputFrame& f) { return f.button_leftstick; })
			.add_getter<bool>("button_rightstick", [](const InputFrame& f) { return f.button_rightstick; })
			.add_getter<bool>("button_leftshoulder", [](const InputFrame& f) { return f.button_leftshoulder; })
			.add_getter<bool>("button_rightshoulder", [](const InputFrame& f) { return f.button_rightshoulder; })
			.add_getter<bool>("button_dpad_up", [](const InputFrame& f) { return f.button_dpad_up; })
			.add_getter<bool>("button_dpad_down", [](const InputFrame& f) { return f.button_dpad_down; })
			.add_getter<bool>("button_dpad_left", [](const InputFrame& f) { return f.button_dpad_left; })
			.add_getter<bool>("button_dpad_right", [](const InputFrame& f) { return f.button_dpad_right; })
			.add_getter<bool>("button_misc1", [](const InputFrame& f) { return f.button_misc1; })
			.add_getter<bool>("button_paddle1", [](const InputFrame& f) { return f.button_paddle1; })
			.add_getter<bool>("button_paddle2", [](const InputFrame& f) { return f.button_paddle2; })
			.add_getter<bool>("button_paddle3", [](const InputFrame& f) { return f.button_paddle3; })
			.add_getter<bool>("button_paddle4", [](const InputFrame& f) { return f.button_paddle4; })
			.add_getter<bool>("button_touchpad", [](const InputFrame& f) { return f.button_touchpad; })
			;
	}

	void
	on_update(float dt) override
	{
		input.update(dt);
		input.start_new_frame();
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

	void on_added_controller(SDL_GameController* controller) override
	{
		input.add_controller(controller);
	}

	void on_lost_joystick_instance(int instance_id) override
	{
		input.lost_controller(instance_id);
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
	bool call_imgui = true;
	auto physfs = Physfs(argv[0]);
	
	if(argc == 2)
	{
		fs::path folder = argv[1];
		folder = fs::canonical(folder);
		physfs.setup(folder.string());
	}
	else
	{
		physfs.setup(PHYSFS_getBaseDir());
	}

	const auto data = load_game_data("main.json");

	return run_game
	(
		data.title, glm::ivec2{data.width, data.height}, call_imgui, []()
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
