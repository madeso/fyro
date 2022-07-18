#pragma once

namespace render
{

struct OpenglStates
{
    std::optional<bool> cull_face;
    std::optional<bool> blending;
    std::optional<bool> depth_test;

    OpenglStates& with_cull_face(bool t);
    OpenglStates& with_blending(bool t);
    OpenglStates& with_depth_test(bool t);
};


void setup_opengl_debug();

void opengl_setup(OpenglStates* states);
void opengl_set2d(OpenglStates* states);
void opengl_set3d(OpenglStates* states);

void apply(OpenglStates* current_states, const OpenglStates& new_states);

}
