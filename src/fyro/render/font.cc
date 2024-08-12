#include "fyro/render/font.h"

#include "stb_truetype.h"
#include "stb_rect_pack.h"

#include <iostream>

#include "fyro/render/texture.h"
#include "fyro/render/render2.h"

#include "fyro/dependencies/dependency_imgui.h"

namespace render
{

/*
//   Include this file in whatever places need to refer to it. In ONE C/C++
//   file, write:
//      #define STB_TRUETYPE_IMPLEMENTATION
//   before the #include of this file. This expands out the actual
//   implementation into that C/C++ file.
//
//   To make the implementation private to the file that generates the implementation,
//      #define STBTT_STATIC
//
//   Simple 3D API (don't ship this, but it's fine for tools and quick start)
//           stbtt_BakeFontBitmap()               -- bake a font to a bitmap for use as texture
//           stbtt_GetBakedQuad()                 -- compute quad to draw for a given char
//
//   Improved 3D API (more shippable):
//           #include "stb_rect_pack.h"           -- optional, but you really want it
//           stbtt_PackBegin()
//           stbtt_PackSetOversampling()          -- for improved quality on small fonts
//           stbtt_PackFontRanges()               -- pack and renders
//           stbtt_PackEnd()
//           stbtt_GetPackedQuad()
//
//   "Load" a font file from a memory buffer (you have to keep the buffer loaded)
//           stbtt_InitFont()
//           stbtt_GetFontOffsetForIndex()        -- indexing for TTC font collections
//           stbtt_GetNumberOfFonts()             -- number of fonts for TTC font collections
//
//   Render a unicode codepoint to a bitmap
//           stbtt_GetCodepointBitmap()           -- allocates and returns a bitmap
//           stbtt_MakeCodepointBitmap()          -- renders into bitmap you provide
//           stbtt_GetCodepointBitmapBox()        -- how big the bitmap must be
//
//   Character advance/positioning
//           stbtt_GetCodepointHMetrics()
//           stbtt_GetFontVMetrics()
//           stbtt_GetFontVMetricsOS2()
//           stbtt_GetCodepointKernAdvance()
*/

struct TextPrinter
{
	float xx;
	float yy;
	float scale;
	Texture* texture;
	SpriteBatch* batch;
	stbtt_packedchar* packedchar;
	int pw;
	int ph;

	void get_packed_quad(
		int char_index, float* xpos, float* ypos, stbtt_aligned_quad* q, int align_to_integer
	)
	{
		float ipw = 1.0f / static_cast<float>(pw);
		float iph = 1.0f / static_cast<float>(ph);
		const stbtt_packedchar* b = packedchar + char_index;

		if (align_to_integer)
		{
			float x = *xpos;  // (float) STBTT_ifloor((*xpos + b->xoff) + 0.5f);
			float y = *ypos;  // (float) STBTT_ifloor((*ypos + b->yoff) + 0.5f);
			q->x0 = x;
			q->y0 = y;
			q->x1 = x + (b->xoff2 - b->xoff) * scale;
			q->y1 = y + (b->yoff2 - b->yoff) * scale;
		}
		else
		{
			q->x0 = *xpos + (b->xoff) * scale;
			q->y0 = *ypos + (b->yoff) * scale;
			q->x1 = *xpos + (b->xoff2) * scale;
			q->y1 = *ypos + (b->yoff2) * scale;
		}

		q->s0 = b->x0 * ipw;
		q->t1 = b->y0 * iph;
		q->s1 = b->x1 * ipw;
		q->t0 = b->y1 * iph;

		*xpos += b->xadvance * scale;
	}

	void print_string(const glm::vec4& color, const std::string& text)
	{
		for (auto c: text)
		{
			if (c >= 32)  // && *text < 128)
			{
				print_single_char(color, c);
			}
		}
	}

	void print_single_char(const glm::vec4& color, char c)
	{
		stbtt_aligned_quad q;
		//stbtt_GetPackedQuad(packed_char, 512, 512, c-32, &xx, &yy, &q, 0);
		get_packed_quad(c - 32, &xx, &yy, &q, 1);
		batch->quad(
			texture,
			Vertex3{{q.x0, q.y0, 0.0f}, color, {q.s0, q.t0}},
			Vertex3{{q.x1, q.y0, 0.0f}, color, {q.s1, q.t0}},
			Vertex3{{q.x1, q.y1, 0.0f}, color, {q.s1, q.t1}},
			Vertex3{{q.x0, q.y1, 0.0f}, color, {q.s0, q.t1}}
		);
	}
};


template<class>
constexpr bool inline always_false_v = false;

struct FontImpl
{
	// ASCII 32..126 is 95 glyphs
	stbtt_packedchar packed_char[96];
	stbtt_bakedchar cdata[96];
	std::unique_ptr<Texture> texture;
	float original_height;
	int image_width;
	int image_height;

	void imgui()
	{
		ImGui::Image(
			reinterpret_cast<void*>(static_cast<intptr_t>(texture->id)), ImVec2{300.0f, 300.0f}
		);
	}

	bool init(const unsigned char* ttf_buffer, float text_height)
	{
		int texture_width = 512;
		int texture_height = 512;

		// If skip=0, then codepoints without a glyph recived the font's "missing character" glyph, typically an empty box by convention.
		const int skip = 0;

		std::vector<unsigned char> alpha_only_pixels;
		alpha_only_pixels.resize(static_cast<std::size_t>(texture_width * texture_height));

		{
			stbtt_pack_context context;
			const auto begin_status = stbtt_PackBegin(
				&context, alpha_only_pixels.data(), texture_width, texture_height, 0, 0, nullptr
			);
			if (0 == begin_status)
			{
				return false;
			}

			stbtt_PackSetSkipMissingCodepoints(&context, skip);
			const auto packed_status
				= stbtt_PackFontRange(&context, ttf_buffer, 0, text_height, 32, 95, packed_char);
			stbtt_PackEnd(&context);

			if (packed_status != 0)
			{
				return false;
			}
		}

		std::vector<unsigned char> rgba_pixels;
		rgba_pixels.resize(static_cast<std::size_t>(texture_width * texture_height * 4));
		for (std::size_t y = 0; y < static_cast<std::size_t>(texture_height); y += 1)
		{
			for (std::size_t x = 0; x < static_cast<std::size_t>(texture_width); x += 1)
			{
				const auto c = alpha_only_pixels[static_cast<std::size_t>(texture_width) * y + x];
				const auto i = (static_cast<std::size_t>(texture_width) * y + x) * 4;
				rgba_pixels[i + 0] = 255;
				rgba_pixels[i + 1] = 255;
				rgba_pixels[i + 2] = 255;
				rgba_pixels[i + 3] = c;
			}
		}

		this->texture = std::make_unique<Texture>(
			rgba_pixels.data(),
			texture_width,
			texture_height,
			TextureEdge::clamp,
			TextureRenderStyle::pixel,
			Transparency::include
		);
		this->original_height = text_height;
		this->image_width = texture_width;
		this->image_height = texture_height;

		return true;
	}

	void print(
		SpriteBatch* batch, float height, float x, float y, const std::vector<TextCommand>& text
	)
	{
		auto color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		const float scale = height / original_height;

		auto printer = TextPrinter{
			x, y, scale, texture.get(), batch, packed_char, image_width, image_height
		};

		for (auto c: text)
		{
			std::visit(
				[&printer, &color](auto&& arg)
				{
					using T = std::decay_t<decltype(arg)>;
					if constexpr (std::is_same_v<T, glm::vec4>)
					{
						color = arg;
					}
					else if constexpr (std::is_same_v<T, std::string>)
					{
						printer.print_string(color, arg);
					}
					else
					{
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				},
				c
			);
		}
	}
};

Font::Font(const unsigned char* ttf_buffer, float text_height)
	: impl(std::make_unique<FontImpl>())
{
	impl->init(ttf_buffer, text_height);
}

Font::~Font() = default;

void Font::print(
	SpriteBatch* batch, float height, float x, float y, const std::vector<TextCommand>& text
)
{
	impl->print(batch, height, x, y, text);
}

void Font::imgui()
{
	impl->imgui();
}

}  //  namespace render
