embed(demo_letter
    AS_BINARY
        ../data/sprites/letter_g.png
)
add_executable(demo WIN32 MACOSX_BUNDLE demo.cc ${demo_letter})
target_link_libraries(
    demo
    PUBLIC tred sprites_cards
    PRIVATE project_options project_warnings
)
target_include_directories(demo
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)



add_executable(minesweeper WIN32 MACOSX_BUNDLE minesweeper.cc)
target_link_libraries(
    minesweeper
    PUBLIC tred sprites_onebit
    PRIVATE project_options project_warnings
)
target_include_directories(minesweeper
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(4d WIN32 MACOSX_BUNDLE 4thdimension.cc)
target_link_libraries(
    4d
    PUBLIC tred sprites_onebit
    PRIVATE project_options project_warnings
)
