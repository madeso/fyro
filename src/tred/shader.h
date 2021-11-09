#pragma once

#include <string_view>

#include "tred/dependency_glm.h"

#include "tred/vertex_layout.h"
#include "tred/uniform.h"


struct ShaderProgram
{
    ShaderProgram
    (
        std::string_view vertex_source,
        std::string_view fragment_source,
        const CompiledVertexLayout& layout
    );

    void
    use() const;

    void
    cleanup();

    Uniform
    get_uniform(const std::string& name) const;

    // shader needs to be bound
    void
    set_float(const Uniform& uniform, float value) const;

    void
    set_vec3(const Uniform& uniform, float x, float y, float z);

    void
    set_vec3(const Uniform& uniform, const glm::vec3& v);

    void
    set_vec4(const Uniform& uniform, float x, float y, float z, float w);

    void
    set_vec4(const Uniform& uniform, const glm::vec4& v);

    void
    set_texture(const Uniform& uniform);

    void
    set_mat(const Uniform& uniform, const glm::mat4& mat);

    void
    set_mat(const Uniform& uniform, const glm::mat3& mat);


    unsigned int shader_program;
    VertexTypes vertex_types;
};


void
setup_textures(ShaderProgram* shader, std::vector<Uniform*> uniform_list);

