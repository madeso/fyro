#pragma once

#include <vector>
#include <optional>

#include "tred/dependency_glm.h"
#include "tred/texture.h"
#include "tred/types.h"
#include "tred/rect.h"


struct Texture;
struct ShaderProgram;

struct Render2;

struct Vertex2
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
    void quad(std::optional<Texture*> texture, const Rectf& scr, const std::optional<Rectf>& texturecoord, const glm::vec4& tint = glm::vec4(1.0f));
    void quad(std::optional<Texture*> texture, const Rectf& scr, const Recti& texturecoord, const glm::vec4& tint = glm::vec4(1.0f));

    void submit();
};
