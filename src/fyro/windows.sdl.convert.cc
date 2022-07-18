#include "fyro/windows.sdl.convert.h"

#include "fyro/assert.h"


namespace sdl
{


glm::ivec2 get_hat_values(Uint8 hat)
{
	switch (hat)
	{
		case SDL_HAT_LEFTUP: return glm::ivec2(-1, 1);
		case SDL_HAT_UP: return glm::ivec2(0, 1);
		case SDL_HAT_RIGHTUP: return glm::ivec2(1, 1);
		case SDL_HAT_LEFT: return glm::ivec2(-1, 0);
		case SDL_HAT_CENTERED: return glm::ivec2(0, 0);
		case SDL_HAT_RIGHT: return glm::ivec2(1, 0);
		case SDL_HAT_LEFTDOWN: return glm::ivec2(-1, -1);
		case SDL_HAT_DOWN: return glm::ivec2(0, -1);
		case SDL_HAT_RIGHTDOWN: return glm::ivec2(1, -1);
		default:
			DIE("Invalid hat value");
			return glm::ivec2(0, 0);
	}
}


input::KeyboardKey to_keyboard_key(SDL_Keysym key)
{
	switch (key.sym)
	{
		case SDLK_RETURN: return input::KeyboardKey::return_key;
		case SDLK_ESCAPE: return input::KeyboardKey::escape;
		case SDLK_BACKSPACE: return input::KeyboardKey::backspace;
		case SDLK_TAB: return input::KeyboardKey::tab;
		case SDLK_SPACE: return input::KeyboardKey::space;
		case SDLK_EXCLAIM: return input::KeyboardKey::exclaim;
		case SDLK_QUOTEDBL: return input::KeyboardKey::quotedbl;
		case SDLK_HASH: return input::KeyboardKey::hash;
		case SDLK_PERCENT: return input::KeyboardKey::percent;
		case SDLK_DOLLAR: return input::KeyboardKey::dollar;
		case SDLK_AMPERSAND: return input::KeyboardKey::ampersand;
		case SDLK_QUOTE: return input::KeyboardKey::quote;
		case SDLK_LEFTPAREN: return input::KeyboardKey::left_paren;
		case SDLK_RIGHTPAREN: return input::KeyboardKey::right_paren;
		case SDLK_ASTERISK: return input::KeyboardKey::asterix;
		case SDLK_PLUS: return input::KeyboardKey::plus;
		case SDLK_COMMA: return input::KeyboardKey::comma;
		case SDLK_MINUS: return input::KeyboardKey::minus;
		case SDLK_PERIOD: return input::KeyboardKey::period;
		case SDLK_SLASH: return input::KeyboardKey::slash;
		case SDLK_0: return input::KeyboardKey::num_0;
		case SDLK_1: return input::KeyboardKey::num_1;
		case SDLK_2: return input::KeyboardKey::num_2;
		case SDLK_3: return input::KeyboardKey::num_3;
		case SDLK_4: return input::KeyboardKey::num_4;
		case SDLK_5: return input::KeyboardKey::num_5;
		case SDLK_6: return input::KeyboardKey::num_6;
		case SDLK_7: return input::KeyboardKey::num_7;
		case SDLK_8: return input::KeyboardKey::num_8;
		case SDLK_9: return input::KeyboardKey::num_9;
		case SDLK_COLON: return input::KeyboardKey::colon;
		case SDLK_SEMICOLON: return input::KeyboardKey::semicolon;
		case SDLK_LESS: return input::KeyboardKey::less;
		case SDLK_EQUALS: return input::KeyboardKey::equals;
		case SDLK_GREATER: return input::KeyboardKey::greater;
		case SDLK_QUESTION: return input::KeyboardKey::question;
		case SDLK_AT: return input::KeyboardKey::at;
		case SDLK_LEFTBRACKET: return input::KeyboardKey::left_bracket;
		case SDLK_BACKSLASH: return input::KeyboardKey::backslash;
		case SDLK_RIGHTBRACKET: return input::KeyboardKey::right_bracket;
		case SDLK_CARET: return input::KeyboardKey::caret;
		case SDLK_UNDERSCORE: return input::KeyboardKey::underscore;
		case SDLK_BACKQUOTE: return input::KeyboardKey::backquote;
		case SDLK_a: return input::KeyboardKey::a;
		case SDLK_b: return input::KeyboardKey::b;
		case SDLK_c: return input::KeyboardKey::c;
		case SDLK_d: return input::KeyboardKey::d;
		case SDLK_e: return input::KeyboardKey::e;
		case SDLK_f: return input::KeyboardKey::f;
		case SDLK_g: return input::KeyboardKey::g;
		case SDLK_h: return input::KeyboardKey::h;
		case SDLK_i: return input::KeyboardKey::i;
		case SDLK_j: return input::KeyboardKey::j;
		case SDLK_k: return input::KeyboardKey::k;
		case SDLK_l: return input::KeyboardKey::l;
		case SDLK_m: return input::KeyboardKey::m;
		case SDLK_n: return input::KeyboardKey::n;
		case SDLK_o: return input::KeyboardKey::o;
		case SDLK_p: return input::KeyboardKey::p;
		case SDLK_q: return input::KeyboardKey::q;
		case SDLK_r: return input::KeyboardKey::r;
		case SDLK_s: return input::KeyboardKey::s;
		case SDLK_t: return input::KeyboardKey::t;
		case SDLK_u: return input::KeyboardKey::u;
		case SDLK_v: return input::KeyboardKey::v;
		case SDLK_w: return input::KeyboardKey::w;
		case SDLK_x: return input::KeyboardKey::x;
		case SDLK_y: return input::KeyboardKey::y;
		case SDLK_z: return input::KeyboardKey::z;
		case SDLK_CAPSLOCK: return input::KeyboardKey::capslock;
		case SDLK_F1: return input::KeyboardKey::f1;
		case SDLK_F2: return input::KeyboardKey::f2;
		case SDLK_F3: return input::KeyboardKey::f3;
		case SDLK_F4: return input::KeyboardKey::f4;
		case SDLK_F5: return input::KeyboardKey::f5;
		case SDLK_F6: return input::KeyboardKey::f6;
		case SDLK_F7: return input::KeyboardKey::f7;
		case SDLK_F8: return input::KeyboardKey::f8;
		case SDLK_F9: return input::KeyboardKey::f9;
		case SDLK_F10: return input::KeyboardKey::f10;
		case SDLK_F11: return input::KeyboardKey::f11;
		case SDLK_F12: return input::KeyboardKey::f12;
		case SDLK_PRINTSCREEN: return input::KeyboardKey::printscreen;
		case SDLK_SCROLLLOCK: return input::KeyboardKey::scrolllock;
		case SDLK_PAUSE: return input::KeyboardKey::pause;
		case SDLK_INSERT: return input::KeyboardKey::insert;
		case SDLK_HOME: return input::KeyboardKey::home;
		case SDLK_PAGEUP: return input::KeyboardKey::pageup;
		case SDLK_DELETE: return input::KeyboardKey::delete_key;
		case SDLK_END: return input::KeyboardKey::end;
		case SDLK_PAGEDOWN: return input::KeyboardKey::pagedown;
		case SDLK_RIGHT: return input::KeyboardKey::right;
		case SDLK_LEFT: return input::KeyboardKey::left;
		case SDLK_DOWN: return input::KeyboardKey::down;
		case SDLK_UP: return input::KeyboardKey::up;
		case SDLK_NUMLOCKCLEAR: return input::KeyboardKey::numlock_clear;
		case SDLK_KP_DIVIDE: return input::KeyboardKey::keypad_divide;
		case SDLK_KP_MULTIPLY: return input::KeyboardKey::keypad_multiply;
		case SDLK_KP_MINUS: return input::KeyboardKey::keypad_minus;
		case SDLK_KP_PLUS: return input::KeyboardKey::keypad_plus;
		case SDLK_KP_ENTER: return input::KeyboardKey::keypad_enter;
		case SDLK_KP_1: return input::KeyboardKey::keypad_1;
		case SDLK_KP_2: return input::KeyboardKey::keypad_2;
		case SDLK_KP_3: return input::KeyboardKey::keypad_3;
		case SDLK_KP_4: return input::KeyboardKey::keypad_4;
		case SDLK_KP_5: return input::KeyboardKey::keypad_5;
		case SDLK_KP_6: return input::KeyboardKey::keypad_6;
		case SDLK_KP_7: return input::KeyboardKey::keypad_7;
		case SDLK_KP_8: return input::KeyboardKey::keypad_8;
		case SDLK_KP_9: return input::KeyboardKey::keypad_9;
		case SDLK_KP_0: return input::KeyboardKey::keypad_0;
		case SDLK_KP_PERIOD: return input::KeyboardKey::keypad_period;
		case SDLK_APPLICATION: return input::KeyboardKey::application;
		case SDLK_POWER: return input::KeyboardKey::power;
		case SDLK_KP_EQUALS: return input::KeyboardKey::keypad_equals;
		case SDLK_F13: return input::KeyboardKey::f13;
		case SDLK_F14: return input::KeyboardKey::f14;
		case SDLK_F15: return input::KeyboardKey::f15;
		case SDLK_F16: return input::KeyboardKey::f16;
		case SDLK_F17: return input::KeyboardKey::f17;
		case SDLK_F18: return input::KeyboardKey::f18;
		case SDLK_F19: return input::KeyboardKey::f19;
		case SDLK_F20: return input::KeyboardKey::f20;
		case SDLK_F21: return input::KeyboardKey::f21;
		case SDLK_F22: return input::KeyboardKey::f22;
		case SDLK_F23: return input::KeyboardKey::f23;
		case SDLK_F24: return input::KeyboardKey::f24;
		case SDLK_EXECUTE: return input::KeyboardKey::execute;
		case SDLK_HELP: return input::KeyboardKey::help;
		case SDLK_MENU: return input::KeyboardKey::menu;
		case SDLK_SELECT: return input::KeyboardKey::select;
		case SDLK_STOP: return input::KeyboardKey::stop;
		case SDLK_AGAIN: return input::KeyboardKey::again;
		case SDLK_UNDO: return input::KeyboardKey::undo;
		case SDLK_CUT: return input::KeyboardKey::cut;
		case SDLK_COPY: return input::KeyboardKey::copy;
		case SDLK_PASTE: return input::KeyboardKey::paste;
		case SDLK_FIND: return input::KeyboardKey::find;
		case SDLK_MUTE: return input::KeyboardKey::mute;
		case SDLK_VOLUMEUP: return input::KeyboardKey::volume_up;
		case SDLK_VOLUMEDOWN: return input::KeyboardKey::volume_down;
		case SDLK_KP_COMMA: return input::KeyboardKey::keypad_comma;
		case SDLK_KP_EQUALSAS400: return input::KeyboardKey::keypad_equals_as_400;
		case SDLK_ALTERASE: return input::KeyboardKey::alt_erase;
		case SDLK_SYSREQ: return input::KeyboardKey::sysreq;
		case SDLK_CANCEL: return input::KeyboardKey::cancel;
		case SDLK_CLEAR: return input::KeyboardKey::clear;
		case SDLK_PRIOR: return input::KeyboardKey::prior;
		case SDLK_RETURN2: return input::KeyboardKey::second_return;
		case SDLK_SEPARATOR: return input::KeyboardKey::separator;
		case SDLK_OUT: return input::KeyboardKey::out;
		case SDLK_OPER: return input::KeyboardKey::oper;
		case SDLK_CLEARAGAIN: return input::KeyboardKey::clear_again;
		case SDLK_CRSEL: return input::KeyboardKey::crsel;
		case SDLK_EXSEL: return input::KeyboardKey::exsel;
		case SDLK_KP_00: return input::KeyboardKey::keypad_00;
		case SDLK_KP_000: return input::KeyboardKey::keypad_000;
		case SDLK_THOUSANDSSEPARATOR: return input::KeyboardKey::thousand_separator;
		case SDLK_DECIMALSEPARATOR: return input::KeyboardKey::decimal_separator;
		case SDLK_CURRENCYUNIT: return input::KeyboardKey::currency_unit;
		case SDLK_CURRENCYSUBUNIT: return input::KeyboardKey::currency_subunit;
		case SDLK_KP_LEFTPAREN: return input::KeyboardKey::keypad_leftparen;
		case SDLK_KP_RIGHTPAREN: return input::KeyboardKey::keypad_rightparen;
		case SDLK_KP_LEFTBRACE: return input::KeyboardKey::keypad_leftbrace;
		case SDLK_KP_RIGHTBRACE: return input::KeyboardKey::keypad_rightbrace;
		case SDLK_KP_TAB: return input::KeyboardKey::keypad_tab;
		case SDLK_KP_BACKSPACE: return input::KeyboardKey::keypad_backspace;
		case SDLK_KP_A: return input::KeyboardKey::keypad_a;
		case SDLK_KP_B: return input::KeyboardKey::keypad_b;
		case SDLK_KP_C: return input::KeyboardKey::keypad_c;
		case SDLK_KP_D: return input::KeyboardKey::keypad_d;
		case SDLK_KP_E: return input::KeyboardKey::keypad_e;
		case SDLK_KP_F: return input::KeyboardKey::keypad_f;
		case SDLK_KP_XOR: return input::KeyboardKey::keypad_xor;
		case SDLK_KP_POWER: return input::KeyboardKey::keypad_power;
		case SDLK_KP_PERCENT: return input::KeyboardKey::keypad_percent;
		case SDLK_KP_LESS: return input::KeyboardKey::keypad_less;
		case SDLK_KP_GREATER: return input::KeyboardKey::keypad_greater;
		case SDLK_KP_AMPERSAND: return input::KeyboardKey::keypad_ampersand;
		case SDLK_KP_DBLAMPERSAND: return input::KeyboardKey::keypad_double_ampersand;
		case SDLK_KP_VERTICALBAR: return input::KeyboardKey::keypad_vertical_bar;
		case SDLK_KP_DBLVERTICALBAR: return input::KeyboardKey::keypad_double_verticle_bar;
		case SDLK_KP_COLON: return input::KeyboardKey::keypad_colon;
		case SDLK_KP_HASH: return input::KeyboardKey::keypad_hash;
		case SDLK_KP_SPACE: return input::KeyboardKey::keypad_space;
		case SDLK_KP_AT: return input::KeyboardKey::keypad_at;
		case SDLK_KP_EXCLAM: return input::KeyboardKey::keypad_exclam;
		case SDLK_KP_MEMSTORE: return input::KeyboardKey::keypad_mem_store;
		case SDLK_KP_MEMRECALL: return input::KeyboardKey::keypad_mem_recall;
		case SDLK_KP_MEMCLEAR: return input::KeyboardKey::keypad_mem_clear;
		case SDLK_KP_MEMADD: return input::KeyboardKey::keypad_mem_add;
		case SDLK_KP_MEMSUBTRACT: return input::KeyboardKey::keypad_mem_subtract;
		case SDLK_KP_MEMMULTIPLY: return input::KeyboardKey::keypad_mem_multiply;
		case SDLK_KP_MEMDIVIDE: return input::KeyboardKey::keypad_mem_divide;
		case SDLK_KP_PLUSMINUS: return input::KeyboardKey::keypad_plus_minus;
		case SDLK_KP_CLEAR: return input::KeyboardKey::keypad_clear;
		case SDLK_KP_CLEARENTRY: return input::KeyboardKey::keypad_clear_entry;
		case SDLK_KP_BINARY: return input::KeyboardKey::keypad_binary;
		case SDLK_KP_OCTAL: return input::KeyboardKey::keypad_octal;
		case SDLK_KP_DECIMAL: return input::KeyboardKey::keypad_decimal;
		case SDLK_KP_HEXADECIMAL: return input::KeyboardKey::keypad_hexadecimal;
		case SDLK_LCTRL: return input::KeyboardKey::ctrl_left;
		case SDLK_LSHIFT: return input::KeyboardKey::shift_left;
		case SDLK_LALT: return input::KeyboardKey::alt_left;
		case SDLK_LGUI: return input::KeyboardKey::gui_left;
		case SDLK_RCTRL: return input::KeyboardKey::ctrl_right;
		case SDLK_RSHIFT: return input::KeyboardKey::shift_right;
		case SDLK_RALT: return input::KeyboardKey::alt_right;
		case SDLK_RGUI: return input::KeyboardKey::gui_right;
		case SDLK_MODE: return input::KeyboardKey::mode;
		case SDLK_AUDIONEXT: return input::KeyboardKey::audio_next;
		case SDLK_AUDIOPREV: return input::KeyboardKey::audio_prev;
		case SDLK_AUDIOSTOP: return input::KeyboardKey::audio_stop;
		case SDLK_AUDIOPLAY: return input::KeyboardKey::audio_play;
		case SDLK_AUDIOMUTE: return input::KeyboardKey::audio_mute;
		case SDLK_MEDIASELECT: return input::KeyboardKey::mediaselect;
		case SDLK_WWW: return input::KeyboardKey::www;
		case SDLK_MAIL: return input::KeyboardKey::mail;
		case SDLK_CALCULATOR: return input::KeyboardKey::calculator;
		case SDLK_COMPUTER: return input::KeyboardKey::computer;
		case SDLK_AC_SEARCH: return input::KeyboardKey::ac_search;
		case SDLK_AC_HOME: return input::KeyboardKey::ac_home;
		case SDLK_AC_BACK: return input::KeyboardKey::ac_back;
		case SDLK_AC_FORWARD: return input::KeyboardKey::ac_forward;
		case SDLK_AC_STOP: return input::KeyboardKey::ac_stop;
		case SDLK_AC_REFRESH: return input::KeyboardKey::ac_refresh;
		case SDLK_AC_BOOKMARKS: return input::KeyboardKey::ac_bookmarks;
		case SDLK_BRIGHTNESSDOWN: return input::KeyboardKey::brightness_down;
		case SDLK_BRIGHTNESSUP: return input::KeyboardKey::brightness_up;
		case SDLK_DISPLAYSWITCH: return input::KeyboardKey::displayswitch;
		case SDLK_KBDILLUMTOGGLE: return input::KeyboardKey::keyboard_illum_toggle;
		case SDLK_KBDILLUMDOWN: return input::KeyboardKey::keyboard_illum_down;
		case SDLK_KBDILLUMUP: return input::KeyboardKey::keyboard_illum_up;
		case SDLK_EJECT: return input::KeyboardKey::eject;
		case SDLK_SLEEP: return input::KeyboardKey::sleep;
		default:
			DIE("Invalid keyboard button");
			return input::KeyboardKey::invalid;
	}
}


input::MouseButton to_mouse_button(Uint8 mb)
{
	switch (mb)
	{
		case SDL_BUTTON_LEFT: return input::MouseButton::left;
		case SDL_BUTTON_MIDDLE: return input::MouseButton::middle;
		case SDL_BUTTON_RIGHT: return input::MouseButton::right;
		case SDL_BUTTON_X1: return input::MouseButton::x1;
		case SDL_BUTTON_X2: return input::MouseButton::x2;
		default:
			DIE("Invalid mouse button");
			return input::MouseButton::invalid;
	}
}


input::GamecontrollerButton to_controller_button(SDL_GameControllerButton b)
{
	switch(b)
	{
		case SDL_CONTROLLER_BUTTON_A: return input::GamecontrollerButton::a;
		case SDL_CONTROLLER_BUTTON_B: return input::GamecontrollerButton::b;
		case SDL_CONTROLLER_BUTTON_X: return input::GamecontrollerButton::x;
		case SDL_CONTROLLER_BUTTON_Y: return input::GamecontrollerButton::y;
		case SDL_CONTROLLER_BUTTON_BACK: return input::GamecontrollerButton::back;
		case SDL_CONTROLLER_BUTTON_GUIDE: return input::GamecontrollerButton::guide;
		case SDL_CONTROLLER_BUTTON_START: return input::GamecontrollerButton::start;
		case SDL_CONTROLLER_BUTTON_LEFTSTICK: return input::GamecontrollerButton::stick_left;
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return input::GamecontrollerButton::stick_right;
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return input::GamecontrollerButton::shoulder_left;
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return input::GamecontrollerButton::shoulder_right;
		case SDL_CONTROLLER_BUTTON_DPAD_UP: return input::GamecontrollerButton::dpad_up;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return input::GamecontrollerButton::dpad_down;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return input::GamecontrollerButton::dpad_left;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return input::GamecontrollerButton::dpad_right;
#if SDL_VERSION_ATLEAST(2, 0, 14)
		case SDL_CONTROLLER_BUTTON_MISC1: return input::GamecontrollerButton::misc1;
		case SDL_CONTROLLER_BUTTON_PADDLE1: return input::GamecontrollerButton::paddle1;
		case SDL_CONTROLLER_BUTTON_PADDLE2: return input::GamecontrollerButton::paddle2;
		case SDL_CONTROLLER_BUTTON_PADDLE3: return input::GamecontrollerButton::paddle3;
		case SDL_CONTROLLER_BUTTON_PADDLE4: return input::GamecontrollerButton::paddle4;
		case SDL_CONTROLLER_BUTTON_TOUCHPAD: return input::GamecontrollerButton::touchpad;
#endif
		default:
		return input::GamecontrollerButton::invalid;
	}
}

input::GamecontrollerButton to_controller_button(SDL_GameControllerAxis a)
{
	switch(a)
	{
		case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return input::GamecontrollerButton::trigger_left;
		case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return input::GamecontrollerButton::trigger_right;
		default:
		return input::GamecontrollerButton::invalid;
	}
}

input::GamecontrollerAxis to_controller_axis(SDL_GameControllerAxis a)
{
	switch(a)
	{
		case SDL_CONTROLLER_AXIS_LEFTX: return input::GamecontrollerAxis::left_x;
		case SDL_CONTROLLER_AXIS_LEFTY: return input::GamecontrollerAxis::left_y;
		case SDL_CONTROLLER_AXIS_RIGHTX: return input::GamecontrollerAxis::right_x;
		case SDL_CONTROLLER_AXIS_RIGHTY: return input::GamecontrollerAxis::right_y;
		default: return input::GamecontrollerAxis::invalid;
	}
}


}
