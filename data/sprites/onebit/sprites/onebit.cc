#include "sprites/onebit.h"

#include "onebit.png.h"

namespace onebit
{
	render::Texture load_texture()
	{
		return render::load_image_from_embedded
		(
			ONEBIT_PNG,
			render::TextureEdge::clamp,
			render::TextureRenderStyle::pixel,
			render::Transparency::include
		);
	}
}
