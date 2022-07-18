#pragma once



#include "fyro/dependency_sdl.h"
#include "fyro/input/key.h"
#include "fyro/input/axis.h"


namespace sdl
{
	glm::ivec2 get_hat_values(Uint8 hat);

	input::KeyboardKey to_keyboard_key(SDL_Keysym key);

	input::MouseButton to_mouse_button(Uint8 mb);

	input::GamecontrollerButton to_controller_button(SDL_GameControllerButton b);
	input::GamecontrollerButton to_controller_button(SDL_GameControllerAxis b);
	input::GamecontrollerAxis to_controller_axis(SDL_GameControllerAxis b);
}

