#pragma once

#include "fyro/render/uniform.h"

#include "embed/types.h"
#include "fyro/types.h"

namespace render
{


enum class TextureEdge
{
	clamp,
	repeat
};


enum class TextureRenderStyle
{
	pixel,
	smooth
};


enum class Transparency
{
	include,
	exclude
};

struct Texture
{
	unsigned int id;
	int width;
	int height;

	Texture();	// invalid texture

	// "internal"
	Texture(void* pixel_data, int w, int h, TextureEdge te, TextureRenderStyle trs, Transparency t);

	~Texture();


	Texture(const Texture&) = delete;
	void operator=(const Texture&) = delete;

	Texture(Texture&&);
	void operator=(Texture&&);

	// clears the loaded texture to a invalid texture
	void unload();
};

// set the texture for the specified uniform
void bind_texture(const Uniform& uniform, const Texture& texture);


Texture load_image_from_bytes(
	const unsigned char* image_source,
	int size,
	TextureEdge te,
	TextureRenderStyle trs,
	Transparency t
);

Texture load_image_from_embedded(
	const embedded_binary& image_binary, TextureEdge te, TextureRenderStyle trs, Transparency t
);

Texture load_image_from_color(u32 pixel, TextureEdge te, TextureRenderStyle trs, Transparency t);


}  //  namespace render

std::shared_ptr<render::Texture> load_texture(const std::string& path);
