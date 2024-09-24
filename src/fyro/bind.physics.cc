#include "fyro/bind.physics.h"

#include "lox/lox.h"

#include "fyro/log.h"
#include "fyro/vfs.h"
#include "fyro/io.h"
#include "fyro/exception.h"

// todo(Gustav): rework this...
#include "fyro/bind.render.h"
#include "fyro/bind.h"
#include <tmxlite/Map.hpp>

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

	void update(lox::Interpreter* inter, float dt)
	{
		flicker.update(dt);

		if (on_update)
		{
			on_update->call(inter, {{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(lox::Interpreter* inter, RenderData* data, std::shared_ptr<lox::Object> rc)
	{
		const auto visible = data->visible;

		if (flicker.is_visible == false)
		{
			// tell it to stop rendering
			data->visible = false;
		}

		if (on_render)
		{
			on_render->call(inter, {{rc}});
		}

		data->visible = visible;
	}

	bool is_riding_solid(fyro::Solid*)
	{
		// todo(Gustav): implement this
		return false;
	}

	void get_squished(lox::Interpreter* inter)
	{
		if (on_get_squished)
		{
			on_get_squished->call(inter, {{}});
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

	void update(lox::Interpreter* inter, float dt)
	{
		if (on_update)
		{
			on_update->call(inter, {{lox::make_number_float(static_cast<double>(dt))}});
		}
	}

	void render(lox::Interpreter* inter, std::shared_ptr<lox::Object> rc)
	{
		if (on_render)
		{
			on_render->call(inter, {{rc}});
		}
	}
};

// the actual c++ class dispatching the virtual functions to the base

struct ScriptActorImpl : fyro::Actor
{
	lox::Interpreter* inter;
	std::shared_ptr<ScriptActor> dispatcher;

	explicit ScriptActorImpl(lox::Interpreter* interpreter) : inter(interpreter) {}

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
			return dispatcher->get_squished(inter);
		}
	}

	void update(float dt) override
	{
		assert(dispatcher);
		dispatcher->update(inter, dt);
	}

	void render(RenderData* data, std::shared_ptr<lox::Object> rc) override
	{
		assert(dispatcher);
		dispatcher->render(inter, data, rc);
	}
};

struct ScriptSolidImpl : fyro::Solid
{
	lox::Interpreter* inter;
	std::shared_ptr<ScriptSolid> dispatcher;

	explicit ScriptSolidImpl(lox::Interpreter* interpreter) : inter(interpreter) {}

	void update(float dt) override
	{
		assert(dispatcher);
		dispatcher->update(inter, dt);
	}

	void render(RenderData*, std::shared_ptr<lox::Object> rc) override
	{
		assert(dispatcher);
		dispatcher->render(inter, rc);
	}
};

// the lox variable wrapper

struct ScriptActorBase
{
	explicit ScriptActorBase(lox::Interpreter* inter)
		: impl(std::make_shared<ScriptActorImpl>(inter))
	{
	}

	std::shared_ptr<ScriptActorImpl> impl;
};

struct ScriptSolidBase
{
	explicit ScriptSolidBase(lox::Interpreter* inter)
		: impl(std::make_shared<ScriptSolidImpl>(inter))
	{
	}

	std::shared_ptr<ScriptSolidImpl> impl;
};

struct FixedSolid : fyro::Solid
{
	void update(float) override
	{
	}

	void render(RenderData*, std::shared_ptr<lox::Object>) override
	{
	}
};

ScriptLevel::ScriptLevel()
	: data(std::make_shared<ScriptLevelData>())
{
}

void ScriptLevel::load_tmx(lox::Lox* lox, const std::string& path)
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

	const auto map_height = static_cast<int>(map.getBounds().height);

	// get objects, parse registrered loaders, warn for errors/mismatches

	const auto& layer_array = map.getLayers();
	for (const auto& layer: layer_array)
	{
		if (layer->getType() != tmx::Layer::Type::Object) continue;
		const auto* object_group = static_cast<const tmx::ObjectGroup*>(layer.get());

		// todo(Gustav): move the printing code to a dear imgui inspector
		// LOG_INFO("Found layer {0}", object_group->getName());
		// const auto& props_array = object_group->getProperties();
		// for (const auto& prop: props_array)
		// {
		// 	LOG_INFO("Prop {0}", prop.getName());
		// }

		const auto& tileset_array = map.getTilesets();
		// for (const auto& tileset: tileset_array)
		// {
		// 	LOG_INFO(
		// 		"Found tileset {0}: {1}-{2}",
		// 		tileset.getName(),
		// 		tileset.getFirstGID(),
		// 		tileset.getLastGID()
		// 	);
		// }

		const auto& obj_array = object_group->getObjects();
		for (const auto& obj: obj_array)
		{
			std::size_t tileset_index = 0;
			for (std::size_t search_index = 0; search_index < tileset_array.size();
				 search_index += 1)
			{
				if (obj.getTileID() < tileset_array[search_index].getFirstGID()) break;
				tileset_index = search_index;
			}

			const auto tileset_name = tileset_array[tileset_index].getName();
			const auto from_tileset_found = data->from_tileset.find(tileset_name);
			if (from_tileset_found != data->from_tileset.end())
			{
				const auto pos = obj.getPosition();
				// todo(Gustav): why are x and y doubles? shouldn't they be integers?
				const auto fpx = static_cast<int>(pos.x);
				const auto fpy = static_cast<int>(pos.y);
				// LOG_INFO("Spawning {0} at {1} {2}", tileset_name, fpx, fpy);
				const auto px = lox::make_number_int(fpx);
				const auto py = lox::make_number_int(map_height - fpy);
				from_tileset_found->second->call(lox->get_interpreter(), {{px, py}});
			}
			else
			{
				// todo(Gustav): provide a detailed error of possible matches and badly spelled names
				LOG_WARNING(
					"Found object called '{2}' #{0} of type '{1}' with tile #{3} from tileset #{4} '{5}' with no callback",
					obj.getUID(),
					obj.getType(),
					obj.getName(),
					obj.getTileID(),
					tileset_index,
					tileset_name
				);
			}
		}
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
	level->level.render(rend->data.get(), rend.instance);
}

}  //  namespace script

namespace bind
{

void bind_phys_actor(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<ScriptActorBase>("Actor", [lox](lox::ArgumentHelper& ah) -> ScriptActorBase
		{
			if(ah.complete()) return ScriptActorBase{nullptr};
			return ScriptActorBase{lox->get_interpreter()};
		})
		.add_function(
			"move_x",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float("dx"));
				if(ah.complete()) { return lox::make_nil(); }
				auto r = x.impl->move_x(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			}
		)
		.add_function(
			"move_y",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dist = static_cast<float>(ah.require_float("dy"));
				if(ah.complete()) { return lox::make_nil(); }
				auto r = x.impl->move_y(dist, fyro::no_collision_reaction);
				return lox::make_bool(r);
			}
		)
		.add_function(
			"flicker",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto duration = static_cast<float>(ah.require_float("duration"));
				const auto interval
					= static_cast<float>(ah.require_float("interval"));  // todo(Gustav): expand with optional
				if(ah.complete()) { return lox::make_nil(); }
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
				auto width = to_int(ah.require_int("width"));
				auto height = to_int(ah.require_int("height"));
				if(ah.complete()) { return lox::make_nil(); }
				x.impl->size = Recti{width, height};
				return lox::make_nil();
			}
		)
		.add_function(
			"set_lrud",
			[](ScriptActorBase& x, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto left = to_int(ah.require_int("left"));
				auto right = to_int(ah.require_int("right"));
				auto up = to_int(ah.require_int("up"));
				auto down = to_int(ah.require_int("down"));
				if(ah.complete()) { return lox::make_nil(); }
				x.impl->size = Recti{left, down, right, up};
				return lox::make_nil();
			}
		);
}

void bind_phys_solid(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<ScriptSolidBase>("Solid", [lox](lox::ArgumentHelper& ah) -> ScriptSolidBase
		{
			if(ah.complete()) return ScriptSolidBase{nullptr};
			return ScriptSolidBase{lox->get_interpreter()};
		})
		.add_function(
			"move",
			[](ScriptSolidBase& s, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dx = static_cast<float>(ah.require_float("dx"));
				auto dy = static_cast<float>(ah.require_float("dy"));
				if(ah.complete()) { return lox::make_nil(); }
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
				auto width = to_int(ah.require_int("width"));
				auto height = to_int(ah.require_int("height"));
				if(ah.complete()) { return lox::make_nil(); }
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
				auto inst = ah.require_instance("to_add");
				if(ah.complete()) { return lox::make_nil(); }
				r.add_actor(inst);
				return lox::make_nil();
			}
		)
		.add_function(
			"load_tmx",
			[lox](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto path = ah.require_string("path");
				if(ah.complete()) { return lox::make_nil(); }
				r.load_tmx(lox, path);
				return lox::make_nil();
			}
		)
		.add_function(
			"add_loader_from_tileset",
			[lox](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto name = ah.require_string("tileset_name");
				const auto callback = ah.require_callable("on_add");
				if(ah.complete()) { return lox::make_nil(); }
				LOG_INFO(
					"Registring a callback {0} that takes {1}",
					name,
					callback->get_arg_info(lox->get_interpreter(), callback.get()).arguments
				);
				r.data->from_tileset[name] = callback;
				return lox::make_nil();
			}
		)
		.add_function(
			"add_solid",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto inst = ah.require_instance("solid");
				if(ah.complete()) { return lox::make_nil(); }
				r.add_solid(inst);
				return lox::make_nil();
			}
		)
		.add_function(
			"update",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto dt = static_cast<float>(ah.require_float("dt"));
				if(ah.complete()) { return lox::make_nil(); }
				r.data->level.update(dt);
				return lox::make_nil();
			}
		)
		.add_function(
			"render",
			[](ScriptLevel& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto rend = ah.require_native<RenderArg>("rend");
				if(ah.complete()) { return lox::make_nil(); }
				script::render_level(r.data.get(), rend);
				return lox::make_nil();
			}
		);
}



}  //  namespace bind
