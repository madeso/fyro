set(src_render
	fyro/render/font.cc fyro/render/font.h
	fyro/render/layer2.cc fyro/render/layer2.h
	fyro/render/opengl_utils.cc fyro/render/opengl_utils.h
	fyro/render/render2.cc fyro/render/render2.h
	fyro/render/shader.cc fyro/render/shader.h
	fyro/render/texture.cc fyro/render/texture.h
	fyro/render/uniform.cc fyro/render/uniform.h
	fyro/render/vertex_layout.cc fyro/render/vertex_layout.h
	fyro/render/viewportdef.cc fyro/render/viewportdef.h
	)

set(src_dependencies
	fyro/dependencies/dependency_glad.h
	fyro/dependencies/dependency_glm.cc fyro/dependencies/dependency_glm.h
	fyro/dependencies/dependency_imgui.h
	fyro/dependencies/dependency_opengl.cc fyro/dependencies/dependency_opengl.h
	fyro/dependencies/dependency_sdl.cc fyro/dependencies/dependency_sdl.h
)

set(src_base
	fyro/assert.cc fyro/assert.h
	fyro/cint.cc fyro/cint.h
	fyro/main.sdl.cc fyro/main.sdl.h
	fyro/log.cc fyro/log.h
	fyro/rect.cc fyro/rect.h
	fyro/types.cc fyro/types.h
	fyro/io.cc fyro/io.h
	fyro/input.cc fyro/input.h
	fyro/exception.cc fyro/exception.h
	fyro/vfs.cc fyro/vfs.h
	fyro/cache.cc fyro/cache.h
	fyro/rgb.cc fyro/rgb.h
	fyro/gamedata.cc fyro/gamedata.h
	fyro/sprite.cc fyro/sprite.h
	fyro/game.cc fyro/game.h

	fyro/rendertypes.h
	fyro/bind.h
	fyro/bind.colors.cc fyro/bind.colors.h 
	fyro/bind.physics.cc fyro/bind.physics.h
	fyro/bind.input.cc fyro/bind.input.h 
	fyro/bind.render.cc fyro/bind.render.h

	fyro/collision2.cc fyro/collision2.h
	fyro/tiles.cc fyro/tiles.h

	fyro/main.cc
)

set(src_pch
	fyro/pch.public.h fyro/pch.private.h
)

set(src
	${src_render}
	${src_dependencies}
	${src_base}
	${src_pch}
)

add_executable(fyro ${src})
target_link_libraries(fyro
	PUBLIC
		external::sdl2
		external::opengl
		external::dear_imgui
		fmt::fmt
		stb::image
		stb::truetype
		stb::rect_pack
		external::glm
		embed
		lox::liblox
		external::json
		external::physfs
		external::tmxlite
	PRIVATE
		project_options
		project_warnings
)
target_include_directories(fyro
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
target_precompile_headers(fyro
	PUBLIC fyro/pch.public.h
	PRIVATE fyro/pch.private.h
)

source_group("" FILES ${src})

source_group("dependencies" FILES ${src_dependencies})
source_group("render" FILES ${src_render})
source_group("base" FILES ${src_base})
source_group("pch" FILES ${src_pch})


# set(src_test
# 	fyro/render/vertex_layout.test.cc
# 	../external/catch/main.cc
# )
# add_executable(test ${src_test})
# target_link_libraries(test
# 	PUBLIC catch fyro catchy
# 	PRIVATE project_options project_warnings
# )
