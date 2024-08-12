#pragma once

#include <memory>
#include <variant>

namespace render
{


struct FontImpl;
struct SpriteBatch;

using TextCommand = std::variant<std::string, glm::vec4>;

struct Font
{
	Font(const unsigned char* ttf_buffer, float text_height);
	~Font();

	std::unique_ptr<FontImpl> impl;

	void print(
		SpriteBatch* batch, float height, float x, float y, const std::vector<TextCommand>& text
	);
	void imgui();
};


}  //  namespace render
