#include "sprites/cards.h"

#include "cards.png.h"

namespace cards
{
	render::Texture load_texture()
	{
		return render::load_image_from_embedded
		(
			CARDS_PNG,
			render::TextureEdge::clamp,
			render::TextureRenderStyle::smooth,
			render::Transparency::include
		);
	}
}
