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
#include "fyro/collision2.h"
#include "fyro/tiles.h"

#include "tmxlite/Map.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;


std::string get_dir_from_file(const std::string& path)
{
	if(const auto slash = path.rfind('/'); slash != std::string::npos)
	{
		return path.substr(0, slash-1);
	}
	else
	{
		return path;
	}
}


int to_int(lox::Ti ti)
{
	return static_cast<int>(ti);
}


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

std::optional<std::vector<char>> read_file_to_bytes_or_none(const std::string& path)
{
	auto* file = PHYSFS_openRead(path.c_str());
	if(file == nullptr)
	{
		return std::nullopt;
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


Exception missing_file_exception(const std::string& path)
{
	// todo(Gustav): split path and find files in dir
	return physfs_exception("unable to open file")
		.append("found files")
		.append(physfs_files_in_dir(get_dir_from_file(path)))
		;
}


std::vector<char> read_file_to_bytes(const std::string& path)
{
	if(auto ret = read_file_to_bytes_or_none(path))
	{
		return *ret;
	}
	else
	{
		throw missing_file_exception(path);
	}
}


std::optional<std::string> read_file_to_string_or_none(const std::string& path)
{
	if(auto ret = read_file_to_bytes_or_none(path))
	{
		ret->emplace_back('\0');
		return ret->data();
	}
	else
	{
		return std::nullopt;
	}
}

std::string read_file_to_string(const std::string& path)
{
	if(auto ret = read_file_to_string_or_none(path))
	{
		return *ret;
	}
	else
	{
		throw missing_file_exception(path);
	}
}


std::optional<json> load_json_or_none(const std::string& path)
{
	if(const auto src = read_file_to_string_or_none(path))
	{
		auto parsed = json::parse(*src);
		return parsed; // assigning to a variable and then returning makes gcc happy
	}
	else
	{
		return std::nullopt;
	}
}


struct GameData
{
	std::string title = "fyro";
	int width = 800;
	int height = 600;
};


GameData load_game_data_or_default(const std::string& path)
{
	try
	{
		if(const auto loaded_data = load_json_or_none(path); loaded_data)
		{
			const auto& data = *loaded_data;

			auto r = GameData{};
			r.title  = data["title"] .get<std::string>();
			r.width  = data["width"] .get<int>();
			r.height = data["height"].get<int>();
			return r;
		}
		else
		{
			auto r = GameData{};
			r.title  = "Example";
			r.width  = 800;
			r.height = 600;
			return r;
		}
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

constexpr float rgb_i2f(int i)
{
	return static_cast<float>(i) / 255.0f;
}

struct Rgb
{
	float r;
	float g;
	float b;

	Rgb() = default;

	constexpr Rgb(float ir, float ig, float ib)
		: r(ir)
		, g(ig)
		, b(ib)
	{
	}
	
	constexpr Rgb(int ir, int ig, int ib)
		: r(rgb_i2f(ir))
		, g(rgb_i2f(ig))
		, b(rgb_i2f(ib))
	{
	}
};

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
	Sprite()
		: screen(1.0f, 1.0f)
		, uv(1.0f, 1.0f)
	{
	}
	std::shared_ptr<render::Texture> texture;
	Rectf screen;
	Rectf uv;
};

struct SpriteAnimation
{
	float speed = 0.0f;
	float accum = 0.0f;
	int current_index = 0;
	int total_sprites = 0;

	void setup
	(
		float a_speed,
		int a_total_sprites
	)
	{
		speed = a_speed;
		total_sprites = a_total_sprites;

		// todo(Gustav): randomize!
		// accum = make_random(...);
		// current_index = make_random(...);
	}

	void update(float dt)
	{
		assert(total_sprites >= 0);
		if(total_sprites == 0) { return; }

		accum += dt;
		while(accum > speed)
		{
			accum -= speed;
			current_index += 1;
			current_index = current_index % total_sprites;
		}
	}
};

struct ScriptSprite
{
	std::vector<Sprite> sprites;
	std::shared_ptr<SpriteAnimation> animation; // may be shared between sprites

	ScriptSprite()
		: animation(std::make_shared<SpriteAnimation>())
	{
	}
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
		if(on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(std::shared_ptr<lox::Object> rc)
	{
		if(on_render)
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
		if(on_get_squished)
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
		if(on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(std::shared_ptr<lox::Object> rc)
	{
		if(on_render)
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
		if(dispatcher) { return dispatcher->is_riding_solid(solid); }
		else { return false; }
	}

	void get_squished() override
	{
		if(dispatcher) { return dispatcher->get_squished(); }
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
	void update(float) override {}
	void render(std::shared_ptr<lox::Object>) override {}
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
		if(was_loaded == false ) { throw Exception{{"failed to parse tmx file"}}; }
		data->tiles.load_from_map(map);

		for(const auto& rect: data->tiles.get_collisions())
		{
			auto solid = std::make_shared<FixedSolid>();
			solid->level = &data->level;

			solid->position = glm::ivec2
			{
				static_cast<int>(rect.left),
				static_cast<int>(rect.bottom)
			};
			solid->size = Recti
			{
				static_cast<int>(rect.get_width()),
				static_cast<int>(rect.get_height())
			};

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


struct ExampleGame : public Game
{
	lox::Lox lox;

	GlobalMappings keyboards;
	Input input;

	std::unique_ptr<State> next_state;
	std::unique_ptr<State> state;

	std::vector<std::shared_ptr<render::Font>> loaded_fonts; // todo(Gustav): replace with cache
	Cache<std::string, render::Texture> texture_cache;

	std::vector<std::shared_ptr<SpriteAnimation>> animations;

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

	void bind_named_colors()
	{
		auto rgb = lox.in_package("fyro.rgb");
		rgb->add_native_getter("white", [&]() { return lox.make_native(Rgb{           255, 255, 255 }); });
		rgb->add_native_getter("light_gray", [&]() { return lox.make_native(Rgb{      160, 160, 160 }); });
		rgb->add_native_getter("gray", [&]() { return lox.make_native(Rgb{            127, 127, 127 }); });
		rgb->add_native_getter("dark_gray", [&]() { return lox.make_native(Rgb{       87, 87, 87    }); });
		rgb->add_native_getter("black", [&]() { return lox.make_native(Rgb{           0, 0, 0       }); });
		rgb->add_native_getter("red", [&]() { return lox.make_native(Rgb{             173, 35, 35   }); });
		rgb->add_native_getter("pure_red", [&]() { return lox.make_native(Rgb{        255, 0, 0     }); });
		rgb->add_native_getter("blue", [&]() { return lox.make_native(Rgb{            42, 75, 215   }); });
		rgb->add_native_getter("pure_blue", [&]() { return lox.make_native(Rgb{       0, 0, 255     }); });
		rgb->add_native_getter("light_blue", [&]() { return lox.make_native(Rgb{      157, 175, 255 }); });
		rgb->add_native_getter("normal_blue", [&]() { return lox.make_native(Rgb{     127, 127, 255 }); });
		rgb->add_native_getter("cornflower_blue", [&]() { return lox.make_native(Rgb{ 100, 149, 237 }); });
		rgb->add_native_getter("green", [&]() { return lox.make_native(Rgb{           29, 105, 20   }); });
		rgb->add_native_getter("pure_green", [&]() { return lox.make_native(Rgb{      0, 255, 0     }); });
		rgb->add_native_getter("light_green", [&]() { return lox.make_native(Rgb{     129, 197, 122 }); });
		rgb->add_native_getter("yellow", [&]() { return lox.make_native(Rgb{          255, 238, 51  }); });
		rgb->add_native_getter("pure_yellow", [&]() { return lox.make_native(Rgb{     255, 255, 0   }); });
		rgb->add_native_getter("orange", [&]() { return lox.make_native(Rgb{          255, 146, 51  }); });
		rgb->add_native_getter("pure_orange", [&]() { return lox.make_native(Rgb{     255, 127, 0   }); });
		rgb->add_native_getter("brown", [&]() { return lox.make_native(Rgb{           129, 74, 25   }); });
		rgb->add_native_getter("pure_brown", [&]() { return lox.make_native(Rgb{      250, 75, 0    }); });
		rgb->add_native_getter("purple", [&]() { return lox.make_native(Rgb{          129, 38, 192  }); });
		rgb->add_native_getter("pure_purple", [&]() { return lox.make_native(Rgb{     128, 0, 128   }); });
		rgb->add_native_getter("pink", [&]() { return lox.make_native(Rgb{            255, 205, 243 }); });
		rgb->add_native_getter("pure_pink", [&]() { return lox.make_native(Rgb{       255, 192, 203 }); });
		rgb->add_native_getter("pure_beige", [&]() { return lox.make_native(Rgb{      245, 245, 220 }); });
		rgb->add_native_getter("tan", [&]() { return lox.make_native(Rgb{             233, 222, 187 }); });
		rgb->add_native_getter("pure_tan", [&]() { return lox.make_native(Rgb{        210, 180, 140 }); });
		rgb->add_native_getter("cyan", [&]() { return lox.make_native(Rgb{            41, 208, 208  }); });
		rgb->add_native_getter("pure_cyan", [&]() { return lox.make_native(Rgb{       0, 255, 255   }); });
	}

	void bind_collision()
	{
		auto fyro = lox.in_package("fyro");
		
		lox.in_global_scope()->define_native_class<ScriptActorBase>("Actor")
			.add_function("move_x", [](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float());
				ah.complete();
				auto r = x.impl->move_x(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			})
			.add_function("move_y", [](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float());
				ah.complete();
				auto r = x.impl->move_y(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			})
			.add_property<lox::Ti>("x",      [](ScriptActorBase& x) -> lox::Ti {return x.impl->position.x;}, [](ScriptActorBase& x, lox::Ti v) {x.impl->position.x = to_int(v);})
			.add_property<lox::Ti>("y",      [](ScriptActorBase& x) -> lox::Ti {return x.impl->position.y;}, [](ScriptActorBase& x, lox::Ti v) {x.impl->position.y = to_int(v);})
			.add_getter<lox::Ti>("width",  [](ScriptActorBase& x) -> lox::Ti {return x.impl->size.get_width();})
			.add_getter<lox::Ti>("height", [](ScriptActorBase& x) -> lox::Ti {return x.impl->size.get_height();})
			.add_function("set_size", [](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto width = to_int(ah.require_int());
				auto height = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{width, height};
				return lox::make_nil();
			})
			.add_function("set_lrud", [](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto left = to_int(ah.require_int());
				auto right = to_int(ah.require_int());
				auto up = to_int(ah.require_int());
				auto down = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{left, down, right, up};
				return lox::make_nil();
			})
			;

		lox.in_global_scope()->define_native_class<ScriptSolidBase>("Solid")
			.add_function("move", [](ScriptSolidBase& s, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dx = static_cast<float>(ah.require_float());
				auto dy = static_cast<float>(ah.require_float());
				ah.complete();
				s.impl->Move(dx, dy);
				return lox::make_nil();
			})
			.add_property<lox::Ti>("x",      [](ScriptSolidBase& x) -> lox::Ti {return x.impl->position.x;}, [](ScriptSolidBase& x, lox::Ti v) {x.impl->position.x = to_int(v);})
			.add_property<lox::Ti>("y",      [](ScriptSolidBase& x) -> lox::Ti {return x.impl->position.y;}, [](ScriptSolidBase& x, lox::Ti v) {x.impl->position.y = to_int(v);})
			.add_getter<lox::Ti>("width",  [](ScriptSolidBase& x) -> lox::Ti {return x.impl->size.get_width();})
			.add_getter<lox::Ti>("height", [](ScriptSolidBase& x) -> lox::Ti {return x.impl->size.get_height();})
			.add_function("set_size", [](ScriptSolidBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto width = to_int(ah.require_int());
				auto height = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{width, height};
				return lox::make_nil();
			})
			;

		fyro->define_native_class<ScriptLevel>("Level")
			.add_function("add", [](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto inst = ah.require_instance();
				ah.complete();
				r.add_actor(inst);
				return lox::make_nil();
			})
			.add_function("load_tmx", [](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto path = ah.require_string();
				ah.complete();
				r.load_tmx(path);
				return lox::make_nil();
			})
			.add_function("add_solid", [](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto inst = ah.require_instance();
				ah.complete();
				r.add_solid(inst);
				return lox::make_nil();
			})
			.add_function("update", [](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dt = static_cast<float>(ah.require_float());
				ah.complete();
				r.data->level.update(dt);
				return lox::make_nil();
			})
			.add_function("render", [](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto rend = ah.require_native<RenderArg>();
				ah.complete();
				r.data->tiles.render(*rend->data->layer->batch, rend->data->layer->viewport_aabb_in_worldspace);
				r.data->level.render(rend.instance);
				return lox::make_nil();
			})
			;
	}

	void bind_functions()
	{
		bind_named_colors();
		bind_collision();
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

		fyro->define_native_class<glm::ivec2>("vec2i", [](lox::ArgumentHelper& ah) -> glm::ivec2{
			const auto x = ah.require_int();
			const auto y = ah.require_int();
			ah.complete();
			return glm::ivec2{x, y};
		})
		;

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
				layer.batch->quadf({}, layer.viewport_aabb_in_worldspace, {}, false, {color->r, color->g, color->b, 1.0f});
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
				layer.batch->quadf({}, Rect{width, height}.translate(x, y), {}, false, {color->r, color->g, color->b, 1.0f}
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
			.add_function("sprite", [this](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto texture = ah.require_native<ScriptSprite>();
				// auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float());
				const auto y = static_cast<float>(ah.require_float());
				const auto flip_x = ah.require_bool();
				/* const auto flip_y =*/ ah.require_bool();
				ah.complete();

				if(texture == nullptr) { return nullptr; }

				auto data = r.data;
				if(data == nullptr) { lox::raise_error("must be called inside State.render()"); return nullptr; }
				if(data->layer.has_value() == false) { lox::raise_error("need to setup virtual render area first"); return nullptr; }
				
				render::RenderLayer2& layer = *data->layer;
				animations.emplace_back(texture->animation);
				const auto animation_index = static_cast<std::size_t>(std::max(0, texture->animation->current_index));
				Sprite& sprite = texture->sprites[animation_index];

				// void quad(std::optional<Texture*> texture, const Rectf& scr, const Recti& texturecoord, const glm::vec4& tint = glm::vec4(1.0f));
				const auto tint = glm::vec4(1.0f);
				const auto screen = Rectf{sprite.screen}.translate(x, y);
				layer.batch->quadf(sprite.texture.get(), screen, sprite.uv, flip_x, tint);
				
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
		fyro->define_native_function("sync_sprite_animations", [](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			auto to_sprite_array = [](std::shared_ptr<lox::Array> src) -> std::vector<lox::NativeRef<ScriptSprite>>
			{
				if(src == nullptr) { return {}; }
				std::vector<lox::NativeRef<ScriptSprite>> dst;
				for(auto& v: src->values)
				{
					if(auto native = lox::as_native<ScriptSprite>(v); native)
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
			for(auto& sp: sprite_array)
			{
				if(main_animation == nullptr)
				{
					main_animation = sp->animation;
				}
				else
				{
					// todo(Gustav): verify that animation has same number of frames!
					if(sp->animation->total_sprites != main_animation->total_sprites)
					{
						lox::raise_error("unable to sync: sprite animations have different number of frames");
					}
					sp->animation = main_animation;
				}
			}

			return lox::make_nil();
		});
		fyro->define_native_function("load_sprite", [this](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			auto to_vec2i_array = [](std::shared_ptr<lox::Array> src) -> std::vector<glm::ivec2>
			{
				if(src == nullptr) { return {}; }
				std::vector<glm::ivec2> dst;
				for(auto& v: src->values)
				{
					if(auto native = lox::as_native<glm::ivec2>(v); native)
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

			if(tiles_array.empty())
			{
				// lox::raise_error("sprite array was empty");

				for(int y=0; y < tiles_per_y; y+=1)
				{
					for(int x=0; x < tiles_per_x; x+=1)
					{
						tiles_array.emplace_back(x, y);
					}
				}
			}

			ScriptSprite r;
			r.animation->setup(anim_speed, static_cast<int>(tiles_array.size()));
			for(const auto& tile: tiles_array)
			{
				Sprite s;
				s.texture = texture_cache.get(path);
				const auto iw = static_cast<float>(s.texture->width);
				const auto ih = static_cast<float>(s.texture->height);
				s.screen = Rectf{iw, ih};

				const auto tile_pix_w = iw/static_cast<float>(tiles_per_x);
				const auto tile_pix_h = ih/static_cast<float>(tiles_per_y);
				const auto tile_frac_w = tile_pix_w / iw;
				const auto tile_frac_h = tile_pix_h / ih;
				const auto dx = tile_frac_w * static_cast<float>(tile.x);
				const auto dy = tile_frac_h * static_cast<float>(tile.y);
				s.uv = Rectf{tile_frac_w, tile_frac_h}.translate(dx, dy);
				r.sprites.emplace_back(s);
			}
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

		for(auto& anim: animations)
		{
			anim->update(dt);
		}

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
		animations.clear();
		if(state)
		{
			state->render(rc);
		}
		else
		{
			// todo(Gustav): draw some basic thing here since to state is set?
			auto r = render::with_layer2(rc, render::LayoutData{render::ViewportStyle::extended, 200.0f, 200.0f});
			r.batch->quadf({}, r.viewport_aabb_in_worldspace, {}, false, {0.8, 0.8, 0.8, 1.0f});
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
	auto physfs = Physfs(argv[0]);
	bool call_imgui = false;
	std::optional<std::string> folder_arg;

	for(int index=1; index<argc; index+=1)
	{
		const std::string cmd = argv[index];
		if(cmd == "--imgui")
		{
			call_imgui = true;
		}
		else
		{
			if(folder_arg)
			{
				std::cerr << "Folder already specified as " << *folder_arg << ": " << cmd << "\n";
				return -1;
			}
			else
			{
				folder_arg = cmd;
			}
		}
	}


	
	if(folder_arg)
	{
		fs::path folder = *folder_arg;
		folder = fs::canonical(folder);
		physfs.setup(folder.string());
	}
	else
	{
		physfs.setup(PHYSFS_getBaseDir());
	}

	const auto data = load_game_data_or_default("main.json");

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
