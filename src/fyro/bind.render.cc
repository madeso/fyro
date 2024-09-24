#include "fyro/bind.render.h"

#include "lox/lox.h"
#include "fyro/render/render2.h"
#include "fyro/vfs.h"
#include "fyro/bind.physics.h"
#include "fyro/bind.h"
#include "fyro/rgb.h"

RenderData::RenderData(const render::RenderCommand& rr)
	: rc(rr)
	, focus(0, 0)
	, visible(true)
{
}

RenderData::~RenderData()
{
	if (layer)
	{
		layer->batch->submit();
	}
}

struct ScriptFont
{
	std::shared_ptr<render::Font> font;

	void setup(const std::string& path, float height)
	{
		auto bytes = read_file_to_bytes(path);
		font = std::make_shared<render::Font>(
			reinterpret_cast<const unsigned char*>(bytes.data()), height
		);
	}
};

namespace script
{
void look_at(RenderData* r_data, ScriptLevelData* level_data, float focusx, float focusy)
{
	const bool pixelize = true;
	using TPixelize = int;

	const auto& aabb = r_data->layer->viewport_aabb_in_worldspace;
	const auto center_screen = glm::vec3{0.5f * aabb.get_width(), 0.5f * aabb.get_height(), 0};

	if (auto bounds = level_data->tiles.get_bounds(); bounds)
	{
		focusx = std::max(focusx, bounds->left + center_screen.x);
		focusx = std::min(focusx, bounds->right - center_screen.x);

		focusy = std::min(focusy, bounds->top - center_screen.y);
		focusy = std::max(focusy, bounds->bottom + center_screen.y);
	}

	const auto translation_float = glm::vec3{-focusx, -focusy, 0} + center_screen;
	const auto translation = pixelize
		? glm::vec3
		{
			static_cast<TPixelize>(translation_float.x),
			static_cast<TPixelize>(translation_float.y),
			0
		}
		: translation_float;
	const auto camera_matrix = glm::translate(glm::mat4(1.0f), translation);
	r_data->rc.set_camera(camera_matrix);
	const auto focus_float = glm::vec2{focusx, focusy} - glm::vec2(center_screen);
	r_data->focus
		= pixelize
			? glm::
				  vec2{static_cast<TPixelize>(focus_float.x), static_cast<TPixelize>(focus_float.y)}
			: focus_float;
}

std::vector<render::TextCommand> TextCommand_vector_from_array(lox::Interpreter* inter, lox::Array* script_commands)
{
	std::vector<render::TextCommand> commands;
	for (auto& obj: script_commands->values)
	{
		if (auto color = lox::as_native<Rgb>(obj); color)
		{
			commands.emplace_back(glm::vec4{color->r, color->g, color->b, 1.0f});
		}
		else
		{
			const auto str = obj->to_flat_string(inter, nullptr, lox::ToStringOptions::for_print());
			commands.emplace_back(str);
		}
	}
	return commands;
}

void draw_sprite(
	bool visible,
	render::RenderLayer2& layer,
	std::vector<std::shared_ptr<SpriteAnimation>>& animations,
	ScriptSprite* texture,
	float x,
	float y,
	bool flip_x
)
{
	animations.emplace_back(texture->animation);
	const auto animation_index
		= static_cast<std::size_t>(std::max(0, texture->animation->current_index));
	Sprite& sprite = texture->sprites[animation_index];

	const auto tint = glm::vec4(1.0f);
	const auto screen = Rectf{sprite.screen}.translate(x, y);
	if (visible)
	{
		layer.batch->quadf(sprite.texture.get(), screen, sprite.uv, flip_x, tint);
	}
}

std::vector<glm::ivec2> to_vec2i_array(std::shared_ptr<lox::Array> src)
{
	if (src == nullptr)
	{
		return {};
	}
	std::vector<glm::ivec2> dst;
	for (auto& v: src->values)
	{
		if (auto native = lox::as_native<glm::ivec2>(v); native)
		{
			dst.emplace_back(*native);
		}
		else
		{
			// todo(Gustav): expand lox so this is part of the argument handler eval
			// todo(Gustav): add argument index here for better error handling
			lox::raise_error("element in array is not a vec2i");
		}
	}
	return dst;
}

ScriptSprite load_sprite(
	const std::string& path,
	Cache<std::string, render::Texture>& texture_cache,
	int tiles_per_x,
	int tiles_per_y,
	float anim_speed,
	const std::vector<glm::ivec2>& tiles_array_arg
)
{
	auto tiles_array = tiles_array_arg;
	if (tiles_array.empty())
	{
		// lox::raise_error("sprite array was empty");

		for (int y = 0; y < tiles_per_y; y += 1)
		{
			for (int x = 0; x < tiles_per_x; x += 1)
			{
				tiles_array.emplace_back(x, y);
			}
		}
	}

	ScriptSprite r;
	r.animation->setup(anim_speed, static_cast<int>(tiles_array.size()));
	for (const auto& tile: tiles_array)
	{
		Sprite s;
		s.texture = texture_cache.get(path);
		const auto iw = static_cast<float>(s.texture->width);
		const auto ih = static_cast<float>(s.texture->height);
		s.screen = Rectf{iw, ih};

		const auto tile_pix_w = iw / static_cast<float>(tiles_per_x);
		const auto tile_pix_h = ih / static_cast<float>(tiles_per_y);
		const auto tile_frac_w = tile_pix_w / iw;
		const auto tile_frac_h = tile_pix_h / ih;
		const auto dx = tile_frac_w * static_cast<float>(tile.x);
		const auto dy = tile_frac_h * static_cast<float>(tile.y);
		s.uv = Rectf{tile_frac_w, tile_frac_h}.translate(dx, dy);
		r.sprites.emplace_back(s);
	}
	return r;
}
}  //  namespace script

namespace bind
{


void bind_render_command(lox::Lox* lox, AnimationsArray* animations)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<RenderArg>("RenderCommand")
		.add_function(
			"windowbox",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto width = static_cast<float>(ah.require_float("width"));
				const auto height = static_cast<float>(ah.require_float("height"));
				if(ah.complete()) { return lox::make_nil(); }

				LOX_ERROR(width > 0.0f, "width must be positive");
				LOX_ERROR(height > 0.0f, "height must be positive");
				LOX_ERROR(r.data, "must be called inside State.render()");

				r.data->layer = render::with_layer2(
					r.data->rc, render::LayoutData{render::ViewportStyle::black_bars, width, height}
				);
				return lox::make_nil();
			}
		)
		.add_function(
			"look_at",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto focusx = static_cast<float>(ah.require_float("focus_x"));
				auto focusy = static_cast<float>(ah.require_float("focus_y"));
				auto level = ah.require_native<ScriptLevel>("level");
				if(ah.complete()) { return lox::make_nil(); }

				LOX_ERROR(r.data, "must be called inside State.render()");

				script::look_at(r.data.get(), level->data.get(), focusx, focusy);
				return lox::make_nil();
			}
		)
		.add_function(
			"clear",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto color = ah.require_native<Rgb>("clear_color");
				if(ah.complete()) { return lox::make_nil(); }
				if (color == nullptr)
				{
					return nullptr;
				}

				auto data = r.data;
				LOX_ERROR(data, "must be called inside State.render()");
				LOX_ERROR(data->layer, "need to setup virtual render area first");

				render::RenderLayer2& layer = *data->layer;

				if (r.data->visible)
				{
					layer.batch->quadf(
						{},
						layer.viewport_aabb_in_worldspace.translate(data->focus),
						{},
						false,
						{color->r, color->g, color->b, 1.0f}
					);
				}
				return lox::make_nil();
			}
		)
		.add_function(
			"rect",
			[](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto color = ah.require_native<Rgb>("color");
				const auto x = static_cast<float>(ah.require_float("x"));
				const auto y = static_cast<float>(ah.require_float("y"));
				const auto width = static_cast<float>(ah.require_float("width"));
				const auto height = static_cast<float>(ah.require_float("height"));

				if(ah.complete()) { return lox::make_nil(); }
				LOX_ASSERT(color);
				LOX_ERROR(width > 0.0f, "width must be positive");
				LOX_ERROR(height > 0.0f, "height must be positive");

				auto data = r.data;
				LOX_ERROR(data, "must be called inside State.render()");
				LOX_ERROR(data->layer, "need to setup virtual render area first");

				render::RenderLayer2& layer = *data->layer;

				if (r.data->visible)
				{
					layer.batch->quadf(
						{},
						Rect{width, height}.translate(x, y),
						{},
						false,
						{color->r, color->g, color->b, 1.0f}
					);
				}
				return lox::make_nil();
			}
		)
		.add_function(
			"text",
			[lox](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto font = ah.require_native<ScriptFont>("font");
				const auto height = static_cast<float>(ah.require_float("height"));
				// auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float("x"));
				const auto y = static_cast<float>(ah.require_float("y"));
				//const auto text = ah.require_string();
				const auto script_commands = ah.require_array("commands");

				if(ah.complete()) { return lox::make_nil(); }
				LOX_ASSERT(font);

				auto data = r.data;
				LOX_ERROR(data, "must be called inside State.render()");
				LOX_ERROR(data->layer, "need to setup virtual render area first");

				if (r.data->visible)
				{
					render::RenderLayer2& layer = *data->layer;

					const auto commands
						= script::TextCommand_vector_from_array(lox->get_interpreter(), script_commands.get());
					font->font->print(layer.batch, height, x, y, commands);
				}

				return lox::make_nil();
			}
		)
		.add_function(
			"sprite",
			[animations](RenderArg& r, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				auto texture = ah.require_native<ScriptSprite>("sprite");
				// auto color = ah.require_native<Rgb>();
				const auto x = static_cast<float>(ah.require_float("x"));
				const auto y = static_cast<float>(ah.require_float("y"));
				const auto flip_x = ah.require_bool("flip_x");
				/* const auto flip_y =*/ah.require_bool("flip_y");
				if(ah.complete()) { return lox::make_nil(); }

				LOX_ASSERT(texture);

				auto data = r.data;
				LOX_ERROR(data, "must be called inside State.render()");
				LOX_ERROR(data->layer, "need to setup virtual render area first");

				script::draw_sprite(
					r.data->visible, *data->layer, *animations, texture.get_ptr(), x, y, flip_x
				);

				return lox::make_nil();
			}
		);
}

void bind_font(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<ScriptFont>("Font");
}

void bind_sprite(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_class<ScriptSprite>("Sprite")
		.add_function(
			"set_size",
			[](ScriptSprite& t, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto width = static_cast<float>(ah.require_float("width"));
				const auto height = static_cast<float>(ah.require_float("height"));
				if(ah.complete()) { return lox::make_nil(); }
				for (auto& s: t.sprites)
				{
					s.screen = Rectf{width, height}.set_bottom_left(s.screen.left, s.screen.bottom);
				}
				return lox::make_nil();
			}
		)
		.add_function(
			"align",
			[](ScriptSprite& t, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
			{
				const auto x = static_cast<float>(ah.require_float("x"));
				const auto y = static_cast<float>(ah.require_float("y"));
				if(ah.complete()) { return lox::make_nil(); }
				for (auto& s: t.sprites)
				{
					s.screen = s.screen.set_bottom_left(
						-s.screen.get_width() * x, -s.screen.get_height() * y
					);
				}
				return lox::make_nil();
			}
		);
}

void bind_fun_load_font(lox::Lox* lox, FontCache* loaded_fonts)
{
	auto fyro = lox->in_package("fyro");

	fyro->define_native_function(
		"load_font",
		[lox, loaded_fonts](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto path = ah.require_string("path");
			const auto size = ah.require_int("size");
			if(ah.complete()) { return lox::make_nil(); }
			ScriptFont r;
			r.setup(path, static_cast<float>(size));
			loaded_fonts->emplace_back(r.font);
			return lox->make_native(r);
		}
	);
}

void bind_fun_load_image(lox::Lox* lox, TextureCache* texture_cache)
{
	auto fyro = lox->in_package("fyro");

	fyro->define_native_function(
		"load_image",
		[lox,
		 texture_cache](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto path = ah.require_string("path");
			if(ah.complete()) { return lox::make_nil(); }
			ScriptSprite r;
			Sprite s;
			s.texture = texture_cache->get(path);
			s.screen = Rectf{
				static_cast<float>(s.texture->width), static_cast<float>(s.texture->height)
			};
			r.sprites.emplace_back(s);
			return lox->make_native(r);
		}
	);
}

void bind_fun_load_sprite(lox::Lox* lox, TextureCache* texture_cache)
{
	auto fyro = lox->in_package("fyro");

	fyro->define_native_function(
		"load_sprite",
		[lox,
		 texture_cache](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			const auto path = ah.require_string("path");
			const auto tiles_per_x = static_cast<int>(ah.require_int("tiles_per_x"));
			const auto tiles_per_y = static_cast<int>(ah.require_int("tiles_per_y"));
			const auto anim_speed = static_cast<float>(ah.require_float("anim_speed"));
			auto tiles_array = script::to_vec2i_array(ah.require_array("tiles_array"));
			if(ah.complete()) { return lox::make_nil(); }

			auto r = script::load_sprite(
				path, *texture_cache, tiles_per_x, tiles_per_y, anim_speed, tiles_array
			);
			return lox->make_native(r);
		}
	);
}

void bind_fun_sync_sprite_animations(lox::Lox* lox)
{
	auto fyro = lox->in_package("fyro");
	fyro->define_native_function(
		"sync_sprite_animations",
		[](lox::Callable*, lox::ArgumentHelper& ah) -> std::shared_ptr<lox::Object>
		{
			auto to_sprite_array
				= [](std::shared_ptr<lox::Array> src) -> std::vector<lox::NativeRef<ScriptSprite>>
			{
				if (src == nullptr)
				{
					return {};
				}
				std::vector<lox::NativeRef<ScriptSprite>> dst;
				for (auto& v: src->values)
				{
					if (auto native = lox::as_native<ScriptSprite>(v); native)
					{
						dst.emplace_back(native);
					}
					else
					{
						// todo(Gustav): expand lox so this is part of the argument handler eval
						// todo(Gustav): add argument index here for better error handling
						lox::raise_error("element in array is not a Sprite");
					}
				}
				return dst;
			};
			auto sprite_array = to_sprite_array(ah.require_array("sprite_array"));
			if(ah.complete()) { return lox::make_nil(); }

			std::shared_ptr<SpriteAnimation> main_animation;
			for (auto& sp: sprite_array)
			{
				if (main_animation == nullptr)
				{
					main_animation = sp->animation;
				}
				else
				{
					// todo(Gustav): verify that animation has same number of frames!
					if (sp->animation->total_sprites != main_animation->total_sprites)
					{
						lox::raise_error(
							"unable to sync: sprite animations have different number of frames"
						);
					}
					sp->animation = main_animation;
				}
			}

			return lox::make_nil();
		}
	);
}



}  //  namespace bind
