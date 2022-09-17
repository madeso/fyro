set(src_empty
)

set(src_input
	fyro/input/key.cc fyro/input/key.h
	fyro/input/axis.cc fyro/input/axis.h
)

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
	
set(src_base
	fyro/assert.cc fyro/assert.h
	fyro/cint.cc fyro/cint.h
	fyro/dependency_opengl.cc fyro/dependency_opengl.h
	fyro/dependency_sdl.cc fyro/dependency_sdl.h fyro/dependency_glad.h
	fyro/fyro.h
	fyro/game.cc fyro/game.h
	fyro/log.cc fyro/log.h
	fyro/random.cc fyro/random.h
	fyro/rect.cc fyro/rect.h
	fyro/types.cc fyro/types.h
	fyro/windows.sdl.convert.cc fyro/windows.sdl.convert.h

	fyro/collision2.cc

	fyro/main.cc
)

set(src_pch
	fyro/pch.public.h fyro/pch.private.h
)

set(src
	${src_input}
	${src_empty}
	${src_render}
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

source_group("input" FILES ${src_input})
source_group("empty" FILES ${src_empty})
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
