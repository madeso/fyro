#pragma once

#include "fyro/cache.h"
#include "fyro/render/font.h"
#include "fyro/render/texture.h"
#include "fyro/sprite.h"

using AnimationsArray = std::vector<std::shared_ptr<SpriteAnimation>>;
using TextureCache = Cache<std::string, render::Texture>;

// todo(Gustav): replace with actual cache
using FontCache = std::vector<std::shared_ptr<render::Font>>;
