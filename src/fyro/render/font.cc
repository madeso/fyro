#include "fyro/render/font.h"

#include "stb_truetype.h"
#include "stb_rect_pack.h"

#include <iostream>

#include "fyro/render/texture.h"
#include "fyro/render/render2.h"

#include "fyro/dependency_imgui.h"

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


struct FontImpl
{
 	// ASCII 32..126 is 95 glyphs
	// stbtt_bakedchar cdata[96];
	stbtt_packedchar packed_char[96];
	std::unique_ptr<Texture> texture;

	void imgui()
	{
		ImGui::Image(static_cast<void*>(&texture->id), ImVec2{100.0f, 100.0f});
	}

	void init(const unsigned char* ttf_buffer, float text_height)
	{
		int texture_width = 512;
		int texture_height = 512;

		unsigned char temp_bitmap[512*512];

		stbtt_pack_context context;
		if(0 == stbtt_PackBegin(&context, temp_bitmap, texture_width, texture_height, 0, 0, nullptr)) { throw "failed to begin packing"; }

		stbtt_PackFontRange(&context, ttf_buffer, 0, text_height, 32, 95, packed_char);

		stbtt_PackEnd(&context);

		//unsigned char ttf_buffer[1<<20];
		//fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
		/*
		const auto bake_result = stbtt_BakeFontBitmap(ttf_buffer, 0, text_height, temp_bitmap, texture_width, texture_height, 32,96, cdata);
		std::cout << "font baking: " << bake_result << "\n";

		if(bake_result == 0)
		{
			std::cout << "failed to bake\n";
			return;
		}*/

		texture = std::make_unique<Texture>(temp_bitmap, texture_width, texture_height, TextureEdge::clamp, TextureRenderStyle::pixel, Transparency::only_alpha);
		/*
		glGenTextures(1, &ftex);
		glBindTexture(GL_TEXTURE_2D, ftex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);
		// can free temp_bitmap at this point
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		*/
	}

	void print(SpriteBatch* batch, const glm::vec4& color, float x, float y, const std::string& text)
	{
		float xx = x;
		float yy = y;
		// const auto color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
		for(auto c: text)
		{
			if (c >= 32)// && *text < 128)
			{
				stbtt_aligned_quad q;
				stbtt_GetPackedQuad(packed_char, 512, 512, c-32, &xx, &yy, &q, 0);
				const auto height = q.t1 - q.t0;
				std::cout << "height " << height << "\n";
				batch->quad
				(
					texture.get(),
					Vertex2{{q.x0, q.y0, 0.0f}, color, {q.s0, q.t0}},
					Vertex2{{q.x1, q.y0, 0.0f}, color, {q.s1, q.t0}},
					Vertex2{{q.x1, q.y1, 0.0f}, color, {q.s1, q.t1}},
					Vertex2{{q.x0, q.y1, 0.0f}, color, {q.s0, q.t1}}
				);
				/*
				stbtt_aligned_quad q;
				auto gbq = [](const stbtt_bakedchar *chardata, int pw, int ph, int char_index, float *xpos, float *ypos, stbtt_aligned_quad *q, int opengl_fillrule)
				{
					float d3d_bias = opengl_fillrule ? 0 : -0.5f;
					float ipw = 1.0f / static_cast<float>(pw), iph = 1.0f / static_cast<float>(ph);
					const stbtt_bakedchar *b = chardata + char_index;
					float round_x = static_cast<float>(static_cast<int>((*xpos + b->xoff) + 0.5f));
					float round_y = static_cast<float>(static_cast<int>((*ypos + b->yoff) + 0.5f));

					q->x0 = round_x + d3d_bias;
					q->y0 = round_y + d3d_bias;
					q->x1 = round_x + b->x1 - b->x0 + d3d_bias;
					q->y1 = round_y + b->y1 - b->y0 + d3d_bias;

					q->s0 = b->x0 * ipw;
					q->t0 = b->y0 * iph;
					q->s1 = b->x1 * ipw;
					q->t1 = b->y1 * iph;

					const auto h = q->t0 - q->t1;
					q->t0 += h;
					q->t1 += h;

					*xpos += b->xadvance;
				};
				// stbtt_GetBakedQuad(cdata, texture->width, texture->height, c-32, &x,&y,&q,0);//1=opengl & d3d10+,0=d3d9
				gbq(cdata, texture->width, texture->height, c-32, &x,&y,&q,0);//1=opengl & d3d10+,0=d3d9
				batch->quad
				(
					texture.get(),
					Vertex2{{q.x0, q.y0, 0.0f}, color, {q.s0, 1.0f - q.t0}},
					Vertex2{{q.x1, q.y0, 0.0f}, color, {q.s1, 1.0f - q.t0}},
					Vertex2{{q.x1, q.y1, 0.0f}, color, {q.s1, 1.0f - q.t1}},
					Vertex2{{q.x0, q.y1, 0.0f}, color, {q.s0, 1.0f - q.t1}}
				);
				*/
			}
		}
	}
};



Font::Font(const unsigned char* ttf_buffer, float text_height)
	: impl(std::make_unique<FontImpl>())
{
	impl->init(ttf_buffer, text_height);
}

Font::~Font() = default;

void Font::print(SpriteBatch* batch, const glm::vec4& color, float x, float y, const std::string& text)
{
	impl->print(batch, color, x, y, text);
}

void Font::imgui()
{
	impl->imgui();
}

}
