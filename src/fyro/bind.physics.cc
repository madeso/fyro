#include "fyro/bind.physics.h"

#include "lox/lox.h"

#include "fyro/vfs.h"
#include "fyro/io.h"
#include "fyro/exception.h"

// todo(Gustav): rework this...
#include "fyro/bind.render.h"
#include "fyro/bind.h"

int to_int(lox::Ti ti)
{
	return static_cast<int>(ti);
}

struct ActiveFlicker
{
	float duration;
	float interval;
	float dt;
};

struct FlickerStatus
{
	bool is_visible = true;
	std::optional<ActiveFlicker> active_flicker;

	void update(float dt)
	{
		if (! active_flicker) return;

		active_flicker->duration -= dt;
		if (active_flicker->duration <= 0)
		{
			active_flicker = std::nullopt;
			is_visible = true;
			// todo(Gustav): call callback if defined
			return;
		}
		active_flicker->dt += dt;
		while (active_flicker->dt >= active_flicker->interval)
		{
			active_flicker->dt -= active_flicker->interval;
			is_visible = ! is_visible;
		}
	}

	void start_flicker(float duration, float interval)
	{
		ActiveFlicker a;
		a.duration = duration;
		a.interval = interval;
		a.dt = 0;

		// todo(Gustav): call callback if defined?
		active_flicker = a;
	}
};

// c++ wrappers over the specific lox class

struct ScriptActor
{
	std::shared_ptr<lox::Instance> instance;

	std::shared_ptr<lox::Callable> on_update;
	std::shared_ptr<lox::Callable> on_render;
	std::shared_ptr<lox::Callable> on_get_squished;

	FlickerStatus flicker;

	ScriptActor(std::shared_ptr<lox::Instance> in)
		: instance(in)
	{
		on_update = instance->get_bound_method_or_null("update");
		on_render = instance->get_bound_method_or_null("render");
		on_get_squished = instance->get_bound_method_or_null("get_squished");
	}

	void update(float dt)
	{
		flicker.update(dt);

		if (on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(std::shared_ptr<lox::Object> rc)
	{
		if (flicker.is_visible == false)
		{
			// todo(Gustav): should we call render with a dummy arg or set a state to keep animating?
			return;
		}

		if (on_render)
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
		if (on_get_squished)
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
		if (on_update)
		{
			on_update->call({{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(std::shared_ptr<lox::Object> rc)
	{
		if (on_render)
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
		if (dispatcher)
		{
			return dispatcher->is_riding_solid(solid);
		}
		else
		{
			return false;
		}
	}

	void get_squished() override
	{
		if (dispatcher)
		{
			return dispatcher->get_squished();
		}
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

struct FixedSolid : fyro::Solid
{
	void update(float) override
	{
	}

	void render(std::shared_ptr<lox::Object>) override
	{
	}
};

ScriptLevel::ScriptLevel()
	: data(std::make_shared<ScriptLevelData>())
{
}

void ScriptLevel::load_tmx(const std::string& path)
{
	auto source = read_file_to_string(path);
	tmx::Map map;
	const auto was_loaded = map.loadFromString(source, get_dir_from_file(path));
	if (was_loaded == false)
	{
		throw Exception{{"failed to parse tmx file"}};
	}
	data->tiles.load_from_map(map);

	for (const auto& rect: data->tiles.get_collisions())
	{
		auto solid = std::make_shared<FixedSolid>();
		solid->level = &data->level;

		solid->position = glm::ivec2{static_cast<int>(rect.left), static_cast<int>(rect.bottom)};
		solid->size
			= Recti{static_cast<int>(rect.get_width()), static_cast<int>(rect.get_height())};

		data->level.solids.emplace_back(solid);
	}
}

void ScriptLevel::add_actor(std::shared_ptr<lox::Instance> x)
{
	auto dispatcher = std::make_shared<ScriptActor>(x);
	auto actor = lox::get_derived<ScriptActorBase>(x);
	actor->impl->dispatcher = dispatcher;
	actor->impl->level = &data->level;
	data->level.actors.emplace_back(actor->impl);
}

void ScriptLevel::add_solid(std::shared_ptr<lox::Instance> x)
{
	auto dispatcher = std::make_shared<ScriptSolid>(x);
	auto solid = lox::get_derived<ScriptSolidBase>(x);
	solid->impl->dispatcher = dispatcher;
	solid->impl->level = &data->level;
	data->level.solids.emplace_back(solid->impl);
}

namespace script
{

void render_level(ScriptLevelData* level, lox::NativeRef<RenderArg> rend)
{
	level->tiles.render(*rend->data->layer->batch, rend->data->layer->viewport_aabb_in_worldspace);
	level->level.render(rend.instance);
}

}  //  namespace script

namespace bind
{

void bind_phys_actor(lox::Lox* lox)
{
	// todo(Gustav): expand script to allow inherit from namespace.Class
	lox->in_global_scope()
		->define_native_class<ScriptActorBase>("Actor")
		.add_function(
			"move_x",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float());
				ah.complete();
				auto r = x.impl->move_x(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			}
		)
		.add_function(
			"move_y",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float());
				ah.complete();
				auto r = x.impl->move_y(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			}
		)
		.add_function(
			"flicker",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto duration = static_cast<float>(ah.require_float());
				const auto interval
					= static_cast<float>(ah.require_float());  // todo(Gustav): expand with optional
				ah.complete();
				LOX_ERROR(duration > 0, "duration must be larger than 0");
				LOX_ERROR(interval > 0, "interval must be larger than 0");
				x.impl->dispatcher->flicker.start_flicker(duration, interval);
				return lox::make_nil();
			}
		)
		.add_property<lox::Ti>(
			"x",
			[](ScriptActorBase& x) -> lox::Ti { return x.impl->position.x; },
			[](ScriptActorBase& x, lox::Ti v) { x.impl->position.x = to_int(v); }
		)
		.add_property<lox::Ti>(
			"y",
			[](ScriptActorBase& x) -> lox::Ti { return x.impl->position.y; },
			[](ScriptActorBase& x, lox::Ti v) { x.impl->position.y = to_int(v); }
		)
		.add_getter<lox::Ti>(
			"width", [](ScriptActorBase& x) -> lox::Ti { return x.impl->size.get_width(); }
		)
		.add_getter<lox::Ti>(
			"height", [](ScriptActorBase& x) -> lox::Ti { return x.impl->size.get_height(); }
		)
		.add_function(
			"set_size",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto width = to_int(ah.require_int());
				auto height = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{width, height};
				return lox::make_nil();
			}
		)
		.add_function(
			"set_lrud",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto left = to_int(ah.require_int());
				auto right = to_int(ah.require_int());
				auto up = to_int(ah.require_int());
				auto down = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{left, down, right, up};
				return lox::make_nil();
			}
		);
}

void bind_phys_solid(lox::Lox* lox)
{
	lox->in_global_scope()
		->define_native_class<ScriptSolidBase>("Solid")
		.add_function(
			"move",
			[](ScriptSolidBase& s, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dx = static_cast<float>(ah.require_float());
				auto dy = static_cast<float>(ah.require_float());
				ah.complete();
				s.impl->Move(dx, dy);
				return lox::make_nil();
			}
		)
		.add_property<lox::Ti>(
			"x",
			[](ScriptSolidBase& x) -> lox::Ti { return x.impl->position.x; },
			[](ScriptSolidBase& x, lox::Ti v) { x.impl->position.x = to_int(v); }
		)
		.add_property<lox::Ti>(
			"y",
			[](ScriptSolidBase& x) -> lox::Ti { return x.impl->position.y; },
			[](ScriptSolidBase& x, lox::Ti v) { x.impl->position.y = to_int(v); }
		)
		.add_getter<lox::Ti>(
			"width", [](ScriptSolidBase& x) -> lox::Ti { return x.impl->size.get_width(); }
		)
		.add_getter<lox::Ti>(
			"height", [](ScriptSolidBase& x) -> lox::Ti { return x.impl->size.get_height(); }
		)
		.add_function(
			"set_size",
			[](ScriptSolidBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto width = to_int(ah.require_int());
				auto height = to_int(ah.require_int());
				ah.complete();
				x.impl->size = Recti{width, height};
				return lox::make_nil();
			}
		);
}

void bind_phys_level(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<ScriptLevel>("Level")
		.add_function(
			"add",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto inst = ah.require_instance();
				ah.complete();
				r.add_actor(inst);
				return lox::make_nil();
			}
		)
		.add_function(
			"load_tmx",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto path = ah.require_string();
				ah.complete();
				r.load_tmx(path);
				return lox::make_nil();
			}
		)
		.add_function(
			"add_solid",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto inst = ah.require_instance();
				ah.complete();
				r.add_solid(inst);
				return lox::make_nil();
			}
		)
		.add_function(
			"update",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dt = static_cast<float>(ah.require_float());
				ah.complete();
				r.data->level.update(dt);
				return lox::make_nil();
			}
		)
		.add_function(
			"render",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto rend = ah.require_native<RenderArg>();
				ah.complete();
				script::render_level(r.data.get(), rend);
				return lox::make_nil();
			}
		);
}



}  //  namespace bind
