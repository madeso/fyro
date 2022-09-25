#pragma once

#include "fyro/render/shader.h"

#include "fyro/render/texture.h"
#include "fyro/types.h"
#include "fyro/rect.h"

namespace render
{

struct Texture;
struct ShaderProgram;
struct Render2;

struct Vertex2
{
	glm::vec2 position;
	glm::vec4 color;
	glm::vec2 texturecoord;
};
struct Vertex3
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec2 texturecoord;
};

struct SpriteBatch
{
	static constexpr int max_quads = 100;

	std::vector<float> data;
	int quads = 0;
	Texture* current_texture = nullptr;
	u32 va;
	u32 vb;
	u32 ib;
	Render2* render;
	Texture white_texture;

	SpriteBatch(ShaderProgram* shader, Render2* r);
	~SpriteBatch();

	SpriteBatch(const SpriteBatch&) = delete;
	void operator=(const SpriteBatch&) = delete;
	SpriteBatch(SpriteBatch&&) = delete;
	void operator=(SpriteBatch&&) = delete;

	void quad(std::optional<Texture*> texture, const Vertex2& v0, const Vertex2& v1, const Vertex2& v2, const Vertex2& v3);
	void quad(std::optional<Texture*> texture, const Vertex3& v0, const Vertex3& v1, const Vertex3& v2, const Vertex3& v3);
	void quadf(std::optional<Texture*> texture, const Rectf& scr, const std::optional<Rectf>& texturecoord, bool flip_x, const glm::vec4& tint = glm::vec4(1.0f));
	void quadi(std::optional<Texture*> texture, const Rectf& scr, const Recti& texturecoord, bool flip_x, const glm::vec4& tint = glm::vec4(1.0f));

	void submit();
};

struct Render2
{
	Render2();

	ShaderVertexAttributes quad_description;
	CompiledShaderVertexAttributes quad_layout;
	ShaderProgram quad_shader;
	Uniform view_projection_uniform;
	Uniform transform_uniform;
	Uniform texture_uniform;

	SpriteBatch batch;
};

}
