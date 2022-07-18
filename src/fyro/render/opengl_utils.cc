#include "fyro/render/opengl_utils.h"

#include "fyro/assert.h"
#include "fyro/log.h"

#include "fyro/dependency_opengl.h"
#include "fyro/dependency_sdl.h"


namespace render
{

const char*
opengl_error_to_string(GLenum error_code)
{
    switch(error_code)
    {
    case GL_INVALID_ENUM: return "INVALID_ENUM"; break;
    case GL_INVALID_VALUE: return "INVALID_VALUE"; break;
    case GL_INVALID_OPERATION: return "INVALID_OPERATION"; break;
#ifdef GL_STACK_OVERFLOW
    case GL_STACK_OVERFLOW: return "STACK_OVERFLOW"; break;
#endif
#ifdef GL_STACK_UNDERFLOW
    case GL_STACK_UNDERFLOW: return "STACK_UNDERFLOW"; break;
#endif
    case GL_OUT_OF_MEMORY: return "OUT_OF_MEMORY"; break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: return "INVALID_FRAMEBUFFER_OPERATION"; break;
    default: return "UNKNOWN"; break;
    }
}


namespace
{
    const char*
    source_to_string(GLenum source)
    {
        switch(source)
        {
        case GL_DEBUG_SOURCE_API_ARB: return "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB: return "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: return "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB: return "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB: return "Application"; break;
        case GL_DEBUG_SOURCE_OTHER_ARB: return "Other"; break;
        default: return "Unknown";
        }
    }

    const char*
    type_to_string(GLenum type)
    {
        switch(type)
        {
        case GL_DEBUG_TYPE_ERROR_ARB: return "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: return "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB: return "Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB: return "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB: return "Performance"; break;
        case GL_DEBUG_TYPE_OTHER_ARB: return "Other"; break;
        default: return "Unknown";
        }
    }

    const char*
    severity_to_string(GLenum severity)
    {
        switch(severity)
        {
        case GL_DEBUG_SEVERITY_HIGH_ARB: return "high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB: return "medium"; break;
        case GL_DEBUG_SEVERITY_LOW_ARB: return "low"; break;
        default: return "unknown";
        }
    }

}  // namespace


void APIENTRY
on_opengl_error
(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei /*length*/,
        const GLchar* message,
        const GLvoid* /*userParam*/
)
{
    // ignore non-significant error/warning codes
    if(type == GL_DEBUG_TYPE_OTHER_ARB)
    {
        return;
    }

    // only display the first 10
    static int ErrorCount = 0;
    if(ErrorCount > 10)
    {
        return;
    }
    ++ErrorCount;

    LOG_ERROR
    (
        "---------------\n"
        "Debug message ({}): {}\n"
        "Source {} type: {} Severity: {}",
        id, message,
        source_to_string(source),
        type_to_string(type),
        severity_to_string(severity)
    );
    // DIE("OpenGL error");
}



void
setup_opengl_debug()
{
    const bool has_debug = GLAD_GL_ARB_debug_output == 1;
    if(has_debug)
    {
        LOG_INFO("Enabling OpenGL debug output");
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(on_opengl_error, nullptr);
        glDebugMessageControlARB
        (
            GL_DONT_CARE,
            GL_DONT_CARE,
            GL_DONT_CARE,
            0,
            nullptr,
            GL_TRUE
        );
    }
}



template<typename T, typename TChange>
void apply_generic_state(std::optional<T>* current_state, std::optional<T> new_state, TChange change_function)
{
    if
    (
        new_state.has_value() == false
        ||
        (current_state->has_value() && **current_state == *new_state)
    )
    {
        return;
    }

    ASSERT(new_state.has_value());

    change_function(*new_state);
    *current_state = new_state;
}

void apply_state(std::optional<bool>* current_state, std::optional<bool> new_state, GLenum gl_type)
{
    apply_generic_state<bool>
    (
        current_state, new_state,
        [gl_type](bool enable)
        {
            if(enable)
            {
                glEnable(gl_type);
            }
            else
            {
                glDisable(gl_type);
            }
        }
    );
}


OpenglStates& OpenglStates::with_cull_face(bool t) { cull_face = t; return *this; }
OpenglStates& OpenglStates::with_blending(bool t) { blending = t; return *this; }
OpenglStates& OpenglStates::with_depth_test(bool t) { depth_test = t; return *this; }


void apply(OpenglStates* current_states, const OpenglStates& new_states)
{
    #define APPLY_STATE(name, gl) apply_state(&current_states->name, new_states.name, gl)
    APPLY_STATE(cull_face, GL_CULL_FACE);
    APPLY_STATE(blending, GL_BLEND);
    APPLY_STATE(depth_test, GL_DEPTH_TEST);
    #undef APPLY_STATE
}



void opengl_setup(OpenglStates* state)
{
    setup_opengl_debug();

    apply
    (
        state,
        OpenglStates{}
            .with_cull_face(true)
    );
    
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void opengl_set2d(OpenglStates* states)
{
    apply
    (
        states,
        OpenglStates{}
            .with_depth_test(false)
            .with_blending(true)
    );
}


void opengl_set3d(OpenglStates* states)
{
    apply
    (
        states,
        OpenglStates{}
            .with_depth_test(true)
            .with_blending(false)
    );
}

}
