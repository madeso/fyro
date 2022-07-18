set(src_empty
)

set(src_input
    tred/input/key.cc tred/input/key.h
    tred/input/axis.cc tred/input/axis.h
)

set(src_render
    tred/render/layer2.cc tred/render/layer2.h
    tred/render/opengl_utils.cc tred/render/opengl_utils.h
    tred/render/render2.cc tred/render/render2.h
    tred/render/shader.cc tred/render/shader.h
    tred/render/texture.cc tred/render/texture.h
    tred/render/uniform.cc tred/render/uniform.h
    tred/render/vertex_layout.cc tred/render/vertex_layout.h
    tred/render/viewportdef.cc tred/render/viewportdef.h
    )
    
set(src_base
    tred/assert.cc tred/assert.h
    tred/cint.cc tred/cint.h
    tred/dependency_opengl.cc tred/dependency_opengl.h
    tred/dependency_sdl.cc tred/dependency_sdl.h tred/dependency_glad.h
    tred/fyro.h
    tred/game.cc tred/game.h
    tred/log.cc tred/log.h
    tred/random.cc tred/random.h
    tred/rect.cc tred/rect.h
    tred/types.cc tred/types.h
    tred/windows.sdl.convert.cc tred/windows.sdl.convert.h
)

set(src_pch
    tred/pch.public.h tred/pch.private.h
)

set(src
    ${src_input}
    ${src_empty}
    ${src_render}
    ${src_base}
    ${src_pch}
)

add_library(tred STATIC ${src})
target_link_libraries(tred
    PUBLIC
        sdl2
        opengl
        imgui
        fmt::fmt
        stb::image
        glm
        embed
    PRIVATE
        project_options
        project_warnings
)
target_include_directories(tred
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
target_precompile_headers(tred
    PUBLIC tred/pch.public.h
    PRIVATE tred/pch.private.h
)

source_group("" FILES ${src})

source_group("input" FILES ${src_input})
source_group("empty" FILES ${src_empty})
source_group("render" FILES ${src_render})
source_group("base" FILES ${src_base})
source_group("pch" FILES ${src_pch})


set(src_test
    tred/render/vertex_layout.test.cc
    ../external/catch/main.cc
)
add_executable(test ${src_test})
target_link_libraries(test
    PUBLIC catch tred catchy
    PRIVATE project_options project_warnings
)
