#include "fyro/input.h"

#include "fyro/log.h"

#include "fyro/dependencies/dependency_imgui.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//// Default mappings

// todo(Gustav): move to a constexpr?
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//// Util functions

float make_stable(float f)
{
	const float ff = f > 0.0f ? f : -f;

	if (ff <= 0.18f)
	{
		return 0.0f;
	}
	else
	{
		return f;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// Capturing

InputFrame capture_keyboard(const KeyboardMapping& mapping)
{
	int numkeys = 0;
	const Uint8* keys = SDL_GetKeyboardState(&numkeys);

	const auto get = [keys, numkeys](std::optional<SDL_Scancode> sc) -> bool
	{
		if (sc)
		{
			if (*sc >= numkeys)
			{
				return false;
			}
			if (*sc < 0)
			{
				return false;
			}
			return keys[*sc] == 1;
		}
		else
		{
			return false;
		}
	};

	const float axis_left_x_neg = get(mapping.axis_left_x_neg) ? -1.0f : 0.0f;
	const float axis_left_x_pos = get(mapping.axis_left_x_pos) ? 1.0f : 0.0f;
	const float axis_left_y_neg = get(mapping.axis_left_y_neg) ? -1.0f : 0.0f;
	const float axis_left_y_pos = get(mapping.axis_left_y_pos) ? 1.0f : 0.0f;

	const float axis_right_x_neg = get(mapping.axis_right_x_neg) ? -1.0f : 0.0f;
	const float axis_right_x_pos = get(mapping.axis_right_x_pos) ? 1.0f : 0.0f;
	const float axis_right_y_neg = get(mapping.axis_right_y_neg) ? -1.0f : 0.0f;
	const float axis_right_y_pos = get(mapping.axis_right_y_pos) ? 1.0f : 0.0f;

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

InputFrame capture_gamecontroller(SDL_GameController* gamecontroller)
{
	// The state is a value ranging from -32768 to 32767.
	const auto from_axis = [gamecontroller](SDL_GameControllerAxis axis) -> float
	{
		const auto s = SDL_GameControllerGetAxis(gamecontroller, axis);
		if (s >= 0)
		{
			return make_stable(static_cast<float>(s) / 32767.0f);
		}
		else
		{
			// don't include sign here as s is already negative
			return make_stable(static_cast<float>(s) / 32768.0f);
		}
	};

	// Triggers, however, range from 0 to 32767 (they never return a negative value).
	const auto from_trigger = [gamecontroller](SDL_GameControllerAxis axis) -> float
	{
		const auto s = SDL_GameControllerGetAxis(gamecontroller, axis);
		union
		{
			Sint16 s;
			Uint16 u;
		} cast;
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//// HapticsEffect

HapticsEffect::HapticsEffect(float f, float l)
	: force(f)
	, life(l)
{
}

bool HapticsEffect::is_alive() const
{
	return life > 0.0f;
}

void HapticsEffect::update(float dt)
{
	life -= dt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// HapticsEngine

HapticsEngine::HapticsEngine(SDL_Joystick* joystick)
	: haptic(create_haptic(joystick))
{
}

HapticsEngine::~HapticsEngine()
{
	if (haptic != nullptr)
	{
		SDL_HapticClose(haptic);
	}
}

void HapticsEngine::on_imgui()
{
	for (std::size_t index = 0; index < effects.size(); index += 1)
	{
		const auto text = "{}"_format(effects[index].life);
		ImGui::TextUnformatted(text.c_str());
	}
}

void HapticsEngine::run(float force, float life)
{
	if (haptic == nullptr)
	{
		return;
	}
	auto e = HapticsEffect{force, life};
	effects.emplace_back(std::move(e));
}

void HapticsEngine::update(float dt)
{
	for (auto& e: effects)
	{
		e.update(dt);
	}

	effects.erase(
		std::remove_if(
			effects.begin(),
			effects.end(),
			[&](const HapticsEffect& e) -> bool { return e.is_alive() == false; }
		),
		effects.end()
	);

	enable_disable_rumble();
}

void HapticsEngine::enable_disable_rumble()
{
	if (haptic == nullptr)
	{
		return;
	}

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
		assert(! last_rumble);
		sdl_start_rumble();
	}
	else if (last_rumble)
	{
		assert(! current);
		sdl_stop_rumble();
	}
	else
	{
		// no rumble action needed
	}

	last_rumble = current;
}

std::optional<float> HapticsEngine::get_rumble_effect() const
{
	std::optional<float> force = std::nullopt;
	for (auto& e: effects)
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

SDL_Haptic* HapticsEngine::create_haptic(SDL_Joystick* joystick)
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
		LOG_WARNING("SDL: rumble not supported: {0}", SDL_GetError());
		SDL_HapticClose(haptic);
		return nullptr;
	}

	return haptic;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// InputDevice_Keyboard

InputDevice_Keyboard::InputDevice_Keyboard(GlobalMappings* m, std::size_t i)
	: mappings(m)
	, index(i)
{
}

std::string InputDevice_Keyboard::get_name()
{
	// todo(Gustav): use format to print index
	return "keyboard";
}

bool InputDevice_Keyboard::is_connected()
{
	return true;
}

InputFrame InputDevice_Keyboard::capture_frame()
{
	const auto r = capture_keyboard(mappings->mappings[index]);
	return r;
}

void InputDevice_Keyboard::run_haptics(float, float)
{
}

void InputDevice_Keyboard::update(float)
{
}

void InputDevice_Keyboard::on_imgui()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// InputDevice_Gamecontroller

InputDevice_Gamecontroller::InputDevice_Gamecontroller(SDL_GameController* c)
	: controller(c)
	, haptics(SDL_GameControllerGetJoystick(c))
	, name(collect_name(c))
{
}

std::string InputDevice_Gamecontroller::collect_name(SDL_GameController* controller)
{
	if (controller == nullptr)
	{
		return "missing controller";
	}

	const char* name = SDL_GameControllerName(controller);
	if (name)
	{
		return name;
	}
	else
	{
		return "<unnamed controller>";
	}
}

InputDevice_Gamecontroller::~InputDevice_Gamecontroller()
{
	clear_controller();
}

void InputDevice_Gamecontroller::clear_controller()
{
	if (controller)
	{
		SDL_GameControllerClose(controller);
		controller = nullptr;
	}
}

int InputDevice_Gamecontroller::get_device_index()
{
	auto* joystick = SDL_GameControllerGetJoystick(controller);
	return SDL_JoystickInstanceID(joystick);
}

std::string InputDevice_Gamecontroller::get_name()
{
	return name;
}

bool InputDevice_Gamecontroller::is_connected()
{
	return controller != nullptr;
}

InputFrame InputDevice_Gamecontroller::capture_frame()
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

void InputDevice_Gamecontroller::run_haptics(float force, float life)
{
	haptics.run(force, life);
}

void InputDevice_Gamecontroller::update(float dt)
{
	haptics.update(dt);
}

void InputDevice_Gamecontroller::on_imgui()
{
	ImGui::TextUnformatted(name.c_str());
	haptics.on_imgui();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// Player

Player::~Player()
{
	if (device)
	{
		device->free = true;
	}
}

void Player::on_imgui()
{
	if (device)
	{
		ImGui::Begin("player");
		device->on_imgui();
		ImGui::End();
	}
}

bool Player::is_connected()
{
	return device && device->is_connected();
}

void Player::update_frame()
{
	last_frame = current_frame;
	current_frame = device ? device->capture_frame() : InputFrame{};
}

void Player::run_haptics(float force, float life)
{
	if (device)
	{
		device->run_haptics(force, life);
	}
}

void Player::update(float dt)
{
	if (device)
	{
		device->update(dt);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// Input

void Input::add_controller(SDL_GameController* controller)
{
	auto ctrl = std::make_shared<InputDevice_Gamecontroller>(controller);
	const auto index = ctrl->get_device_index();
	LOG_INFO("Connected {0}: {1}", index, ctrl->get_name());
	controllers.insert({index, ctrl});
}

void Input::lost_controller(int instance_id)
{
	if (auto found = controllers.find(instance_id); found != controllers.end())
	{
		LOG_INFO("Lost {0}: {1}", found->first, found->second->get_name());
		found->second->clear_controller();
		controllers.erase(found);
	}
}

void Input::on_imgui()
{
	for (auto& p: players)
	{
		p->on_imgui();
	}
}

void Input::add_keyboard(std::shared_ptr<InputDevice_Keyboard> kb)
{
	keyboards.emplace_back(kb);
}

std::shared_ptr<InputDevice> Input::find_device()
{
	for (auto& co: controllers)
	{
		auto& c = co.second;
		if (c->free && c->capture_frame().button_start)
		{
			c->free = false;
			return c;
		}
	}

	for (auto& c: keyboards)
	{
		if (c->free && c->capture_frame().button_start)
		{
			c->free = false;
			return c;
		}
	}

	return nullptr;
}

void Input::start_new_frame()
{
	next_player = 0;

	// todo(Gustav): remove old players

	// age players
	for (auto& p: players)
	{
		p->age += 1;
	}
}

std::shared_ptr<Player> Input::capture_player()
{
	if (next_player >= players.size())
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
		if (r->is_connected() == false)
		{
			r->device = find_device();
		}
		r->update_frame();
		return r;
	}
}

void Input::update(float dt)
{
	for (auto& p: players)
	{
		p->update(dt);
	}
}
