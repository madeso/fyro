#pragma once

#include <memory>

#include "tmxlite/Map.hpp"

#include "fyro/render/render2.h"
#include "fyro/rect.h"

struct MapImpl;

struct Map
{
	std::unique_ptr<MapImpl> impl;

	Map();
	~Map();

	void load_from_map(const tmx::Map& map);
	void update(float dt);
	void render(render::SpriteBatch& batch, const Rectf& view);
};
