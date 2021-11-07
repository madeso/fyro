#include "tred/layer2.h"

#include "tred/render2.h"
#include "tred/spritebatch.h"
#include "tred/dependency_opengl.h"
#include "tred/viewportdef.h"
#include "tred/opengl_state.h"
#include "tred/opengl_utils.h"


void set_gl_viewport(const Recti& r)
{
    glViewport(r.left, r.bottom, r.get_width(), r.get_height());
}


RenderLayer2::~RenderLayer2()
{
    batch->submit();
}


RenderLayer2::RenderLayer2(Layer&& l, SpriteBatch* b)
    : Layer(l)
    , batch(b)
{
}

RenderLayer3::RenderLayer3(Layer&& l)
    : Layer(l)
{
}


Layer create_layer(const ViewportDef& vp)
{
    return {{vp.virtual_width, vp.virtual_height}, vp.screen_rect};
}


RenderLayer2 create_layer2(const RenderCommand& rc, const ViewportDef& vp)
{
    set_gl_viewport(vp.screen_rect);
    opengl_set2d(rc.states);

    const auto camera = glm::mat4(1.0f);
    const auto projection = glm::ortho(0.0f, vp.virtual_width, 0.0f, vp.virtual_height);

    rc.render->quad_shader.use();
    rc.render->quad_shader.set_mat(rc.render->view_projection_uniform, projection);
    rc.render->quad_shader.set_mat(rc.render->transform_uniform, camera);

    // todo(Gustav): transform viewport according to the camera
    return RenderLayer2{create_layer(vp), &rc.render->batch};
}

RenderLayer3 create_layer3(const RenderCommand& rc, const ViewportDef& vp)
{
    set_gl_viewport(vp.screen_rect);
    opengl_set3d(rc.states);

    return RenderLayer3{create_layer(vp)};
}


glm::vec2 Layer::mouse_to_world(const glm::vec2& p) const
{
    // transform from mouse pixels to window 0-1
    const auto n = Cint_to_float(screen).to01(p);
    return viewport_aabb_in_worldspace.from01(n);
}


ViewportDef create_viewport(const LayoutData& ld, const glm::ivec2& size)
{
    if(ld.style==ViewportStyle::black_bars)
    {
        return ViewportDef::fit_with_black_bars(ld.requested_width, ld.requested_height, size.x, size.y);
    }
    else
    {
        return ViewportDef::extend(ld.requested_width, ld.requested_height, size.x, size.y);
    }
}

RenderLayer2 with_layer2(const RenderCommand& rc, const LayoutData& ld)
{
    const auto vp = create_viewport(ld, rc.size);
    return create_layer2(rc, vp);
}

RenderLayer3 with_layer3(const RenderCommand& rc, const LayoutData& ld)
{
    const auto vp = create_viewport(ld, rc.size);
    return create_layer3(rc, vp);
}


Layer with_layer(const InputCommand& rc, const LayoutData& ld)
{
    const auto vp = create_viewport(ld, rc.size);
    return create_layer(vp);
}
