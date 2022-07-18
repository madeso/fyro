#pragma once

#include "tred/rect.h"

namespace render
{

struct ViewportDef
{
    Recti screen_rect;

    float virtual_width;
    float virtual_height;
};


/** The viewport is scaled, with aspect in mind, and centered.
 * Fits the viewport, scaling it, keeping the aspect ratio. Black bars may appear if the aspect ration doesnt match
 */
[[nodiscard]]
ViewportDef
fit_with_black_bars
(
    float width,
    float height,
    int window_width,
    int window_height
);


/** The viewports height or idth is extended to match the screen.
 * Fits the viewport, scaling it to the max and then fits the viewport without stretching. This means that the viewport isnt the same size as requested.
 */
[[nodiscard]]
ViewportDef
extended_viewport(float width, float height, int window_width, int window_height);


}
