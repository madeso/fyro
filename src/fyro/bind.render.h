#pragma once

#include "fyro/render/layer2.h"
#include "fyro/rendertypes.h"

namespace lox
{
struct Lox;
}

struct RenderData
{
	const render::RenderCommand& rc;
	std::optional<render::RenderLayer2> layer;
	glm::vec2 focus;

	explicit RenderData(const render::RenderCommand& rr);
	~RenderData();
};

struct RenderArg
{
	std::shared_ptr<RenderData> data;
};

namespace bind
{

void bind_render_command(lox::Lox* lox, AnimationsArray* animations);
void bind_font(lox::Lox* lox);
void bind_sprite(lox::Lox* lox);
void bind_fun_load_font(lox::Lox* lox, FontCache* loaded_fonts);
void bind_fun_load_image(lox::Lox* lox, TextureCache* texture_cache);
void bind_fun_load_sprite(lox::Lox* lox, TextureCache* texture_cache);
void bind_fun_sync_sprite_animations(lox::Lox* lox);

}  //  namespace bind
