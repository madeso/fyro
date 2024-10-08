#pragma once

#include "lox/lox.h"
#include "fyro/main.sdl.h"
#include "fyro/input.h"
#include "fyro/rendertypes.h"

struct State
{
	State() = default;
	virtual ~State() = default;
	virtual void update(float) = 0;
	virtual void render(const render::RenderCommand& rc) = 0;
};

struct ExampleGame : public Game
{
	lox::Lox lox;
	GlobalMappings keyboards;
	Input input;
	std::unique_ptr<State> next_state;
	std::unique_ptr<State> state;
	FontCache loaded_fonts;
	TextureCache texture_cache;
	AnimationsArray animations;

	ExampleGame();

	void on_imgui() override;

	void run_main();

	void on_update(float dt) override;
	void on_render(const render::RenderCommand& rc) override;
	void on_mouse_position(const render::InputCommand&, const glm::ivec2&) override;
	void on_added_controller(SDL_GameController* controller) override;
	void on_lost_joystick_instance(int instance_id) override;
};
