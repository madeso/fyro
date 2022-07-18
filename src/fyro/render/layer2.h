#pragma once

#include "fyro/rect.h"


namespace render
{

struct Render2;
struct SpriteBatch;
struct OpenglStates;

struct Layer
{
    Rectf viewport_aabb_in_worldspace;
    Recti screen;

    glm::vec2 mouse_to_world(const glm::vec2& p) const;
};

struct RenderLayer2 : Layer
{
    SpriteBatch* batch;

    RenderLayer2(Layer&& l, SpriteBatch* batch);

    ~RenderLayer2();
};

struct RenderLayer3 : Layer
{
    RenderLayer3(Layer&& l);
};

enum class ViewportStyle { black_bars, extended};


struct LayoutData
{
    ViewportStyle style;
    float requested_width;
    float requested_height;
};


struct InputCommand
{
    glm::ivec2 size;
};

struct RenderCommand
{
    OpenglStates* states;
    Render2* render;
    glm::ivec2 size;

    // tood(Gustav): add clear to color function
    void clear(const glm::vec3& color, const LayoutData& ld) const;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Functions


// todo(Gustav): pass camera to with_ functions
// todo(Gustav): move to member functions

RenderLayer2 with_layer2(const RenderCommand& rc, const LayoutData& ld);
RenderLayer3 with_layer3(const RenderCommand& rc, const LayoutData& ld);
Layer with_layer(const InputCommand& rc, const LayoutData& ld);

}
