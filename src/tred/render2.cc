#include "tred/render2.h"


Render2::Render2()
    : quad_description
    (
        {
            {VertexType::position2, "position"},
            {VertexType::color4, "color"},
            {VertexType::texture2, "uv"}
        }
    )
    , quad_layout
    (
        compile({quad_description}).compile(quad_description)
    )
    , quad_shader
    (
        R"glsl(
            #version 450 core
            in vec3 position;
            in vec4 color;
            in vec2 uv;

            uniform mat4 view_projection;
            uniform mat4 transform;

            out vec4 varying_color;
            out vec2 varying_uv;

            void main()
            {
                varying_color = color;
                varying_uv = uv;
                gl_Position = view_projection * transform * vec4(position, 1.0);
            }
        )glsl",
        R"glsl(
            #version 450 core

            in vec4 varying_color;
            in vec2 varying_uv;

            uniform sampler2D uniform_texture;

            out vec4 color;

            void main()
            {
                color = texture(uniform_texture, varying_uv) * varying_color;
            }
        )glsl",
        quad_layout
    )
    , view_projection_uniform(quad_shader.get_uniform("view_projection"))
    , transform_uniform(quad_shader.get_uniform("transform"))
    , texture_uniform(quad_shader.get_uniform("uniform_texture"))
    , batch(&quad_shader, this)
{
    setup_textures(&quad_shader, {&texture_uniform});
}
