#pragma once

#include "lox/lox.h"
#include "fyro/main.sdl.h"
#include "fyro/cache.h"
#include "fyro/input.h"
#include "fyro/render/font.h"
#include "fyro/render/texture.h"
#include "fyro/sprite.h"

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
	std::vector<std::shared_ptr<render::Font>> loaded_fonts;  // todo(Gustav): replace with cache
	Cache<std::string, render::Texture> texture_cache;
	std::vector<std::shared_ptr<SpriteAnimation>> animations;

	ExampleGame();

	void on_imgui() override;

	void run_main();
	void bind_named_colors();
	void bind_collision();
	void bind_functions();

	void on_update(float dt) override;
	void on_render(const render::RenderCommand& rc) override;
	void on_mouse_position(const render::InputCommand&, const glm::ivec2&) override;
	void on_added_controller(SDL_GameController* controller) override;
	void on_lost_joystick_instance(int instance_id) override;
};
