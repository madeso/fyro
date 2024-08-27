#include "fyro/bind.input.h"

#include "fyro/input.h"
#include "lox/lox.h"

struct ScriptPlayer
{
	std::shared_ptr<Player> player;
};

namespace bind
{

void bind_fun_get_input(lox::Lox* lox, Input* input)
{
	auto fyro = lox->in_package("fyro");

	fyro->define_native_function(
		"get_input",
		[lox, input](lox::Callable*, lox::ArgumentHelper& arguments)
		{
			arguments.complete();
			auto frame = input->capture_player();
			return lox->make_native<ScriptPlayer>(ScriptPlayer{frame});
		}
	);
}

void bind_player(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<ScriptPlayer>("Player")
		.add_native_getter<InputFrame>(
			"current",
			[lox](const ScriptPlayer& s) { return lox->make_native(s.player->current_frame); }
		)
		.add_native_getter<InputFrame>(
			"last", [lox](const ScriptPlayer& s) { return lox->make_native(s.player->last_frame); }
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
}

void bind_input_frame(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
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


}  //  namespace bind
