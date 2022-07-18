add_subdirectory(catchy)
add_subdirectory(embed)
add_subdirectory(stb)

###################################################################################################
# glad
add_library(glad STATIC
    glad/src/glad.c
    glad/include/glad/glad.h
)
target_include_directories(glad SYSTEM
    PUBLIC
        glad/include
)
target_link_libraries(glad
    PUBLIC
        ${CMAKE_DL_LIBS}
)


###################################################################################################
# imgui
add_library(imgui STATIC
    imgui/imgui_demo.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/examples/imgui_impl_sdl.cpp
    imgui/examples/imgui_impl_opengl3.cpp
)
target_include_directories(imgui SYSTEM
    PUBLIC
        imgui/
        imgui/examples/
)
target_link_libraries(imgui
    PUBLIC
        glad
        opengl
        sdl2
)
target_compile_definitions(imgui
    PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)


###################################################################################################
# imgui binary_to_compressed
add_library(imgui_binary_to_compressed INTERFACE)
target_include_directories(imgui_binary_to_compressed SYSTEM
    INTERFACE
        imgui-1.78/misc/fonts/
)


###################################################################################################
# SDL2
find_package(SDL2 REQUIRED)
add_library(sdl2 INTERFACE)
target_include_directories(sdl2 SYSTEM
    INTERFACE
        ${SDL2_INCLUDE_DIR}
)
target_link_libraries(sdl2
    INTERFACE
        ${SDL2_LIBRARY}
        ${CMAKE_DL_LIBS}
)


###################################################################################################
# OpenGL
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
add_library(opengl INTERFACE)
target_link_libraries(opengl INTERFACE OpenGL::GL)


###################################################################################################
# glm
add_library(glm INTERFACE)
target_include_directories(glm SYSTEM
    INTERFACE
        glm
)
