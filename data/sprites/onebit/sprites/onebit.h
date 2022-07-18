#pragma once

#include "fyro/render/texture.h"
#include "fyro/rect.h"


#include <array>

namespace onebit
{
    render::Texture load_texture();

	constexpr Recti get(int x, int y) { return Recti::from_xywh(16*x, 16*y, 16, 16); }

    constexpr std::string_view text_string =
        "01234" "56789" ":.%"
        "abcde" "fghij" "klm"
        "nopqr" "stuvw" "xyz"
        "#+-*/" "=@"
        "!?$"
        ;
    constexpr std::array<Recti, 49> text =
    {
        get(35, 17), get(36, 17), get(37, 17), get(38, 17), get(39, 17),
        get(40, 17), get(41, 17), get(42, 17), get(43, 17), get(44, 17),
        get(45, 17), get(46, 17), get(47, 17),

        get(35, 18), get(36, 18), get(37, 18), get(38, 18), get(39, 18),
        get(40, 18), get(41, 18), get(42, 18), get(43, 18), get(44, 18),
        get(45, 18), get(46, 18), get(47, 18),

        get(35, 19), get(36, 19), get(37, 19), get(38, 19), get(39, 19),
        get(40, 19), get(41, 19), get(42, 19), get(43, 19), get(44, 19),
        get(45, 19), get(46, 19), get(47, 19),

        get(35, 20), get(36, 20), get(37, 20), get(38, 20), get(39, 20),
        get(40, 20), get(41, 20),

        get(40, 13), get(42, 13), get(35, 16)
    };

    static_assert(text.size() == text_string.size());

    constexpr Recti bomb = get(45, 9);
    constexpr Recti box = get(39, 14);
    constexpr Recti box_cross = get(40, 14);
    constexpr Recti box_skull = get(41, 14);

    constexpr Recti smiley_happy = get(35, 14);
    constexpr Recti smiley_angry = get(36, 14);
    constexpr Recti smiley_joy = get(37, 14);
    constexpr Recti smiley_sad = get(38, 14);
    constexpr Recti smiley_skull = get(38, 11);

    constexpr Recti n0 = get(35, 17); constexpr Recti n1 = get(36, 17); constexpr Recti n2 = get(37, 17); constexpr Recti n3 = get(38, 17); constexpr Recti n4 = get(39, 17);
    constexpr Recti n5 = get(40, 17); constexpr Recti n6 = get(41, 17); constexpr Recti n7 = get(42, 17); constexpr Recti n8 = get(43, 17); constexpr Recti n9 = get(44, 17);

    constexpr Recti char_a = get(35, 18);

    constexpr Recti cross = get(39, 13);
    constexpr Recti circle = get(40, 13);
}
