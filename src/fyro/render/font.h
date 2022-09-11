#pragma once

#include <memory>

namespace render
{


struct FontImpl;
struct SpriteBatch;


struct Font
{
	Font(const unsigned char* ttf_buffer, float text_height);
	~Font();

	std::unique_ptr<FontImpl> impl;
	
	void print(SpriteBatch* batch, float height, const glm::vec4& color, float x, float y, const std::string& text);
	void imgui();
};


}
