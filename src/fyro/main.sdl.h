#pragma once

#include "SDL.h"

#include <memory>
#include <functional>

#include "fyro/rect.h"
#include "fyro/types.h"

#include "fyro/render/shader.h"
#include "fyro/render/vertex_layout.h"
#include "fyro/render/texture.h"
#include "fyro/render/render2.h"
#include "fyro/render/layer2.h"

enum class MouseButton
{
	invalid,
	unbound	 /// No key
		,
	left  /// The left mouse button
		,
	middle	/// The middle mouse button
		,
	right  /// The right mouse button
		,
	x1	/// The X2 mouse button
		,
	x2	/// The X2 mouse button
};

struct Game
{
	Game() = default;
	virtual ~Game() = default;

	bool run = true;

	virtual void on_render(const render::RenderCommand& rc);
	virtual void on_imgui();
	virtual void on_update(float);

	virtual void on_key(char key, bool down);
	virtual void on_mouse_position(const render::InputCommand&, const glm::ivec2& position);
	virtual void on_mouse_button(const render::InputCommand&, MouseButton button, bool down);
	virtual void on_mouse_wheel(int scroll);

	virtual void on_added_controller(SDL_GameController* controller);
	virtual void on_lost_joystick_instance(int instance_id);
};

int run_game(
	const std::string& title,
	const glm::ivec2& size,
	bool call_imgui,
	std::function<std::shared_ptr<Game>()> make_game
);
