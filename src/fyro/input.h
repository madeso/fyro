#pragma once

#include "fyro/dependencies/dependency_sdl.h"

#include <map>

struct KeyboardMapping
{
	std::optional<SDL_Scancode> axis_left_x_neg;
	std::optional<SDL_Scancode> axis_left_x_pos;
	std::optional<SDL_Scancode> axis_left_y_neg;
	std::optional<SDL_Scancode> axis_left_y_pos;
	std::optional<SDL_Scancode> axis_right_x_neg;
	std::optional<SDL_Scancode> axis_right_x_pos;
	std::optional<SDL_Scancode> axis_right_y_neg;
	std::optional<SDL_Scancode> axis_right_y_pos;

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

struct HapticsEffect
{
	float force;
	float life;

	HapticsEffect(float f, float l);

	bool is_alive() const;
	void update(float dt);
};

struct HapticsEngine
{
	SDL_Haptic* haptic;
	std::vector<HapticsEffect> effects;

	explicit HapticsEngine(SDL_Joystick* joystick);
	~HapticsEngine();

	void on_imgui();
	void run(float force, float life);
	void update(float dt);
	void enable_disable_rumble();

	std::optional<float> last_rumble = std::nullopt;

	std::optional<float> get_rumble_effect() const;

	HapticsEngine(const HapticsEngine&) = delete;
	void operator=(const HapticsEngine&) = delete;
	HapticsEngine(HapticsEngine&&) = delete;
	void operator=(HapticsEngine&&) = delete;

	static SDL_Haptic* create_haptic(SDL_Joystick* joystick);
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

	InputDevice_Keyboard(GlobalMappings* m, std::size_t i);

	std::string get_name() override;
	bool is_connected() override;
	InputFrame capture_frame() override;
	void run_haptics(float, float) override;
	void update(float) override;
	void on_imgui() override;
};

struct InputDevice_Gamecontroller : InputDevice
{
	SDL_GameController* controller;
	HapticsEngine haptics;
	std::string name;

	explicit InputDevice_Gamecontroller(SDL_GameController* c);
	~InputDevice_Gamecontroller();

	static std::string collect_name(SDL_GameController* controller);


	void clear_controller();
	int get_device_index();

	std::string get_name() override;
	bool is_connected() override;
	InputFrame capture_frame() override;
	void run_haptics(float force, float life) override;
	void update(float dt) override;
	void on_imgui() override;
};

struct Player
{
	int age = 0;
	InputFrame current_frame;
	InputFrame last_frame;
	std::shared_ptr<InputDevice> device;

	Player() = default;
	~Player();

	void on_imgui();
	bool is_connected();
	void update_frame();
	void run_haptics(float force, float life);
	void update(float dt);
};

struct Input
{
	std::map<int, std::shared_ptr<InputDevice_Gamecontroller>> controllers;
	std::vector<std::shared_ptr<InputDevice_Keyboard>> keyboards;
	std::vector<std::shared_ptr<Player>> players;
	std::size_t next_player = 0;

	void add_controller(SDL_GameController* controller);
	void lost_controller(int instance_id);
	void on_imgui();
	void add_keyboard(std::shared_ptr<InputDevice_Keyboard> kb);

	// remove lost controller

	std::shared_ptr<InputDevice> find_device();
	void start_new_frame();
	std::shared_ptr<Player> capture_player();
	void update(float dt);
};

KeyboardMapping create_default_mapping_for_player1();
