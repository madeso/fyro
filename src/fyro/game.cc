#include "fyro/game.h"

#include <random>
#include "lox/printhandler.h"

#include "fyro/log.h"
#include "fyro/exception.h"
#include "fyro/vfs.h"

#include "fyro/bind.h"
#include "fyro/bind.colors.h"
#include "fyro/bind.physics.h"
#include "fyro/bind.input.h"
#include "fyro/bind.render.h"

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
		if (on_update)
		{
			on_update->call(lox->get_interpreter(), {{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(const render::RenderCommand& rc) override
	{
		if (on_render)
		{
			auto ra = std::make_shared<RenderData>(rc);
			auto render = lox->make_native<RenderArg>(RenderArg{ra});
			on_render->call(lox->get_interpreter(), {{render}});
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
		: generator(create_engine())
	{
	}

	lox::Ti next_int(lox::Ti max)
	{
		std::uniform_int_distribution<lox::Ti> distribution(0, max - 1);
		return distribution(generator);
	}
};

struct PrintLoxError : lox::PrintHandler
{
	void on_line(std::string_view line) override
	{
		LOG_ERROR("> {0}", line);
	}
};

namespace bind
{

void bind_random(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<ScriptRandom>("Random").add_function(
		"next_int",
		[](ScriptRandom& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto max_value = ah.require_int("max_value");
			if(ah.complete()) { return lox::make_nil(); }
			const auto value = r.next_int(max_value);
			return lox::make_number_int(value);
		}
	);
}

void bind_ivec2(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<glm::ivec2>(
		"vec2i",
		[](lox::ArgumentHelper& ah) -> glm::ivec2
		{
			const auto x = ah.require_int("x");
			const auto y = ah.require_int("y");
			if(ah.complete()) { return glm::ivec2(0, 0); }
			return glm::ivec2{x, y};
		}
	);
}

void bind_fun_set_state(lox::Lox* lox, std::unique_ptr<State>* next_state)
{
	auto fyro = lox->in_package("fyro");

	fyro->define_native_function(
		"set_state",
		[lox, next_state](lox::Callable*, lox::ArgumentHelper& arguments)
		{
			auto instance = arguments.require_instance("next_state");
			if(arguments.complete()) { return lox::make_nil(); }
			*next_state = std::make_unique<ScriptState>(instance, lox);
			return lox::make_nil();
		}
	);
}

}  //  namespace bind

ExampleGame::ExampleGame()
	: lox(std::make_unique<PrintLoxError>(), [](const std::string& str) { LOG_INFO("> {0}", str); })
	, texture_cache([](const std::string& path) { return load_texture(path); })
{
	// todo(Gustav): read/write to json and provide ui for adding new mappings
	keyboards.mappings.emplace_back(create_default_mapping_for_player1());
	input.add_keyboard(std::make_shared<InputDevice_Keyboard>(&keyboards, 0));

	bind::bind_named_colors(&lox);
	bind::bind_rgb(&lox);

	bind::bind_phys_actor(&lox);
	bind::bind_phys_solid(&lox);
	bind::bind_phys_level(&lox);

	bind::bind_fun_set_state(&lox, &next_state);

	bind::bind_fun_get_input(&lox, &input);
	bind::bind_player(&lox);
	bind::bind_input_frame(&lox);

	bind::bind_ivec2(&lox);
	bind::bind_random(&lox);

	bind::bind_render_command(&lox, &animations);
	bind::bind_font(&lox);
	bind::bind_sprite(&lox);

	bind::bind_fun_load_font(&lox, &loaded_fonts);
	bind::bind_fun_load_image(&lox, &texture_cache);
	bind::bind_fun_sync_sprite_animations(&lox);
	bind::bind_fun_load_sprite(&lox, &texture_cache);
}

void ExampleGame::on_imgui()
{
	for (auto& f: loaded_fonts)
	{
		f->imgui();
	}
	input.on_imgui();
}

void ExampleGame::run_main()
{
	const auto src = read_file_to_string("main.lox");
	if (false == lox.run_string(src))
	{
		throw Exception{{"Unable to run script"}};
	}
}

void ExampleGame::on_update(float dt)
{
	input.update(dt);
	input.start_new_frame();

	for (auto& anim: animations)
	{
		anim->update(dt);
	}

	if (state)
	{
		state->update(dt);
	}
	if (next_state != nullptr)
	{
		state = std::move(next_state);
	}
}

void ExampleGame::on_render(const render::RenderCommand& rc)
{
	animations.clear();
	if (state)
	{
		state->render(rc);
	}
	else
	{
		// todo(Gustav): draw some basic thing here since to state is set?
		auto r = render::with_layer2(
			rc, render::LayoutData{render::ViewportStyle::extended, 200.0f, 200.0f}
		);
		r.batch->quadf({}, r.viewport_aabb_in_worldspace, {}, false, {0.8, 0.8, 0.8, 1.0f});
		r.batch->submit();
	}
}

void ExampleGame::on_mouse_position(const render::InputCommand&, const glm::ivec2&)
{
}

void ExampleGame::on_added_controller(SDL_GameController* controller)
{
	input.add_controller(controller);
}

void ExampleGame::on_lost_joystick_instance(int instance_id)
{
	input.lost_controller(instance_id);
}
