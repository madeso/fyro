#pragma once

#include <memory>
#include <optional>

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

	const std::vector<Rectf>& get_collisions() const;
	std::optional<Rectf> get_bounds() const;
};
