# add_subdirectory(catchy)
add_subdirectory(embed)
add_subdirectory(stb)
add_subdirectory(lox)

###################################################################################################
# glad
add_library(external_glad STATIC
    glad/src/glad.c
    glad/include/glad/glad.h
)
target_include_directories(external_glad SYSTEM
    PUBLIC
        glad/include
)
target_link_libraries(external_glad
    PUBLIC
        ${CMAKE_DL_LIBS}
)
add_library(external::glad ALIAS external_glad)


###################################################################################################
# imgui
add_library(external_dear_imgui STATIC
    imgui/imgui_demo.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/examples/imgui_impl_sdl.cpp
    imgui/examples/imgui_impl_opengl3.cpp
)
target_include_directories(external_dear_imgui SYSTEM
    PUBLIC
        imgui/
        imgui/examples/
)
target_link_libraries(external_dear_imgui
    PUBLIC
        external::glad
        external::opengl
        external::sdl2
)
target_compile_definitions(external_dear_imgui
    PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)
add_library(external::dear_imgui ALIAS external_dear_imgui)



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
add_library(external_sdl2 INTERFACE)
target_include_directories(external_sdl2 SYSTEM
    INTERFACE
        ${SDL2_INCLUDE_DIR}
)
target_link_libraries(external_sdl2
    INTERFACE
        ${SDL2_LIBRARY}
        ${CMAKE_DL_LIBS}
)
add_library(external::sdl2 ALIAS external_sdl2)



###################################################################################################
# OpenGL
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)
add_library(external_opengl INTERFACE)
target_link_libraries(external_opengl INTERFACE OpenGL::GL)
add_library(external::opengl ALIAS external_opengl)



###################################################################################################
# glm
add_library(external_glm INTERFACE)
target_include_directories(external_glm SYSTEM
    INTERFACE
        glm
)
add_library(external::glm ALIAS external_glm)



###################################################################################################
# json
add_library(external_json INTERFACE)
target_include_directories(external_json SYSTEM
    INTERFACE
        nlohmann_json/include
)
add_library(external::json ALIAS external_json)




###################################################################################################
# physfs
add_library(external_physfs STATIC
    physfs/src/physfs.c
    physfs/src/physfs_byteorder.c
    physfs/src/physfs_unicode.c
    physfs/src/physfs_platform_posix.c
    physfs/src/physfs_platform_unix.c
    physfs/src/physfs_platform_windows.c
    physfs/src/physfs_platform_os2.c
    physfs/src/physfs_platform_qnx.c
    physfs/src/physfs_platform_android.c
    physfs/src/physfs_archiver_dir.c
    physfs/src/physfs_archiver_unpacked.c
    physfs/src/physfs_archiver_grp.c
    physfs/src/physfs_archiver_hog.c
    physfs/src/physfs_archiver_7z.c
    physfs/src/physfs_archiver_mvl.c
    physfs/src/physfs_archiver_qpak.c
    physfs/src/physfs_archiver_wad.c
    physfs/src/physfs_archiver_zip.c
    physfs/src/physfs_archiver_slb.c
    physfs/src/physfs_archiver_iso9660.c
    physfs/src/physfs_archiver_vdf.c
)

target_compile_definitions(external_physfs PRIVATE "PHYSFS_STATIC")
if(UNIX AND NOT WIN32 AND NOT APPLE)  # (MingW and such might be UNIX _and_ WINDOWS!)
    find_library(PTHREAD_LIBRARY pthread)
    if(PTHREAD_LIBRARY)
		target_link_libraries(external_physfs ${PTHREAD_LIBRARY})
    endif()
endif()

# target_compile_definitions(external_physfs PUBLIC PHYSFS_SUPPORTS_ZIP=0)

target_include_directories(external_physfs SYSTEM
    PUBLIC
        physfs/src
)
add_library(external::physfs ALIAS external_physfs)




###################################################################################################
# tmxlite
add_library(external_tmxlite STATIC
	tmxlite/tmxlite/src/detail/pugixml.cpp
	tmxlite/tmxlite/src/FreeFuncs.cpp
	tmxlite/tmxlite/src/ImageLayer.cpp
	tmxlite/tmxlite/src/Map.cpp
	tmxlite/tmxlite/src/miniz.c
	tmxlite/tmxlite/src/Object.cpp
	tmxlite/tmxlite/src/ObjectGroup.cpp
	tmxlite/tmxlite/src/Property.cpp
	tmxlite/tmxlite/src/TileLayer.cpp
	tmxlite/tmxlite/src/LayerGroup.cpp
	tmxlite/tmxlite/src/Tileset.cpp
	tmxlite/tmxlite/src/ObjectTypes.cpp
)

# target_compile_definitions(external_tmxlite PRIVATE "PHYSFS_STATIC")

target_include_directories(external_tmxlite SYSTEM
    PUBLIC
        tmxlite/tmxlite/include
)
add_library(external::tmxlite ALIAS external_tmxlite)




