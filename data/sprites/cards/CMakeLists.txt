set(src
    sprites/cards.cc sprites/cards.h
)
embed(sprites
    AS_BINARY
        cards.png
)

add_library(sprites_cards STATIC ${src} ${sprites})
target_link_libraries(sprites_cards
    PUBLIC fyro
    PRIVATE project_options project_warnings
)
target_include_directories(sprites_cards
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)
