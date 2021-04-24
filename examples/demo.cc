#include "tred/fyro.h"

struct ExampleGame : public Game
{
    ExampleGame()
    {
        float vertices[] =
        {
            -0.5, -0.5, 0.0f,   0.18f, 0.60f, 0.96f, 1.0f,
             0.5, -0.5, 0.0f,   0.18f, 0.60f, 0.96f, 1.0f,
             0.5,  0.5, 0.0f,   1.00f, 0.93f, 0.24f, 1.0f,
            -0.5,  0.5, 0.0f,   1.00f, 0.93f, 0.24f, 1.0f
        };

        quad_shader.Use();

        glGenVertexArrays(1, &va);
        glBindVertexArray(va);

        glGenBuffers(1, &vb);
        glBindBuffer(GL_ARRAY_BUFFER, vb);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

        u32 indices[] =
        {
            0, 1, 2,
            2, 3, 0
        };

        glGenBuffers(1, &ib);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }

    GLuint va;
    GLuint vb;
    GLuint ib;

    void
    OnRender(const glm::ivec2& size) override
    {
        glViewport(0, 0, size.x, size.y);
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        const auto projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);

        quad_shader.Use();
        quad_shader.SetMat(view_projection_uniform, projection);
        quad_shader.SetMat(transform_uniform, glm::mat4(1.0f));

        glBindVertexArray(va);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
};

int
main(int, char**)
{
    return Run
    (
        "Example", glm::ivec2{800, 600}, false, []()
        {
            return std::make_shared<ExampleGame>();
        }
    );
}

