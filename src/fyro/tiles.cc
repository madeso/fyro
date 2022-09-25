#include "fyro/tiles.h"

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <limits>
#include <iostream>
#include <cmath>
#include <optional>

#include "fyro/render/texture.h"
#include "fyro/render/render2.h"
#include "fyro/rect.h"

std::shared_ptr<render::Texture> load_texture(const std::string& path);

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>


int floor_to_int(float f)
{
	return static_cast<int>(floorf(f));
}


namespace
{
	void flipY(glm::vec2* v0, glm::vec2* v1, glm::vec2* v2, glm::vec2* v3)
	{
		//Flip Y
		glm::vec2 tmp = *v0;
		v0->y = v2->y;
		v1->y = v2->y;
		v2->y = tmp.y ;
		v3->y = v2->y  ;
	}

	void flipX(glm::vec2* v0, glm::vec2* v1, glm::vec2* v2, glm::vec2* v3)
	{
		//Flip X
		glm::vec2 tmp = *v0;
		v0->x = v1->x;
		v1->x = tmp.x;
		v2->x = v3->x;
		v3->x = v0->x ;
	}

	void flipD(glm::vec2*, glm::vec2* v1, glm::vec2*, glm::vec2* v3)
	{
		//Diagonal flip
		glm::vec2 tmp = *v1;
		v1->x = v3->x;
		v1->y = v3->y;
		v3->x = tmp.x;
		v3->y = tmp.y;
	}

	void doFlips(std::uint8_t bits, glm::vec2 *v0, glm::vec2 *v1, glm::vec2 *v2, glm::vec2 *v3)
	{
		//0000 = no change
		//0100 = vertical = swap y axis
		//1000 = horizontal = swap x axis
		//1100 = horiz + vert = swap both axes = horiz+vert = rotate 180 degrees
		//0010 = diag = rotate 90 degrees right and swap x axis
		//0110 = diag+vert = rotate 270 degrees right
		//1010 = horiz+diag = rotate 90 degrees right
		//1110 = horiz+vert+diag = rotate 90 degrees right and swap y axis
		if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			!(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//Shortcircuit tests for nothing to do
			return;
		}
		else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			!(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//0100
			flipY(v0,v1,v2,v3);
		}
		else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			!(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//1000
			flipX(v0,v1,v2,v3);
		}
		else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			!(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//1100
			flipY(v0,v1,v2,v3);
			flipX(v0,v1,v2,v3);
		}
		else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//0010
			flipD(v0,v1,v2,v3);
		}
		else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//0110
			flipX(v0,v1,v2,v3);
			flipD(v0,v1,v2,v3);
		}
		else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//1010
			flipY(v0,v1,v2,v3);
			flipD(v0,v1,v2,v3);
		}
		else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
			(bits & tmx::TileLayer::FlipFlag::Vertical) &&
			(bits & tmx::TileLayer::FlipFlag::Diagonal) )
		{
			//1110
			flipY(v0,v1,v2,v3);
			flipX(v0,v1,v2,v3);
			flipD(v0,v1,v2,v3);
		}
	}
}



struct AnimationState
{
	glm::ivec2 tileCords;
	float startTime;
	float currentTime;
	tmx::Tileset::Tile animTile;
	std::uint8_t flipFlags;
};

using RenderQuad = std::array<render::Vertex2, 4>;

glm::vec2 transform_tile(const glm::vec2& src, const glm::ivec2&)
{
	return
	{
		src.x / 16.0f,
		src.y / 16.0f
	};
}

struct ChunkArray
{
	std::shared_ptr<render::Texture> m_texture;
	glm::ivec2 texSize;

	tmx::Vector2u tileSetSize;
	glm::ivec2 tsTileCount;
	std::uint32_t m_firstGID, m_lastGID;
	std::vector<RenderQuad> tiles;

	ChunkArray(std::shared_ptr<render::Texture> t, const tmx::Tileset& ts)
		: m_texture(t)
		, texSize(t->width, t->height)
	{
		tileSetSize = ts.getTileSize();
		tsTileCount.x = texSize.x / static_cast<int>(tileSetSize.x);
		tsTileCount.y = texSize.y / static_cast<int>(tileSetSize.y);
		m_firstGID = ts.getFirstGID();
		m_lastGID = ts.getLastGID();
	}

	~ChunkArray() = default;
	ChunkArray(const ChunkArray&) = delete;
	ChunkArray& operator = (const ChunkArray&) = delete;

	void reset()
	{
		tiles.clear();
	}
	void addTile(const RenderQuad& tile)
	{
		// std::cout << "adding tile:\n";
		// for(auto& t: tile)
		// {
		// 	std::cout << "    (" << t.texturecoord.x << ", " << t.texturecoord.y << ")\n";
		// }
		tiles.emplace_back(tile);
	}

	void draw(render::SpriteBatch& batch) const
	{
		for(const auto& q: tiles)
		{
			batch.quad(m_texture.get(), q[0], q[1], q[2], q[3]);
		}
	}
};

struct Chunk
{
	glm::vec2 position;
	float layerOpacity;     // opacity of the layer
	glm::vec2 layerOffset;   // Layer offset
	glm::ivec2 mapTileSize;   // general Tilesize of Map
	glm::ivec2 chunkTileCount;   // chunk tilecount
	std::vector<tmx::TileLayer::Tile> m_chunkTileIDs; // stores all tiles in this chunk for later manipulation
	std::vector<glm::vec4> m_chunkColors; // stores colors for extended color effects
	std::map<std::uint32_t, tmx::Tileset::Tile> m_animTiles;    // animation catalogue
	std::vector<AnimationState> m_activeAnimations;     // Animations to be done in this chunk
	std::vector<std::unique_ptr<ChunkArray>> m_chunkArrays;

	void setPosition(const glm::vec2& p) { position = p; }
	glm::vec2 getPosition() const { return position; }

	Chunk
	(
		const tmx::TileLayer& layer,
		std::vector<const tmx::Tileset*> tilesets,
		const glm::vec2& position,
		const glm::ivec2& tileCount,
		const glm::ivec2& tileSize,
		int rowSize,
		const std::map<std::uint32_t, tmx::Tileset::Tile>& animTiles
	)
		: m_animTiles(animTiles)
	{
		setPosition(position);
		layerOpacity = layer.getOpacity() /  1.f * 255.f;
		glm::vec4 vertColour = glm::vec4{200 ,200, 200, layerOpacity};
		auto offset = layer.getOffset();
		layerOffset.x = static_cast<float>(offset.x);
		layerOffset.y = static_cast<float>(offset.y);
		chunkTileCount.x = tileCount.x;
		chunkTileCount.y = tileCount.y;
		mapTileSize = tileSize;
		const auto& tileIDs = layer.getTiles();

		//go through the tiles and create all arrays (for latter manipulation)
		for (const auto& ts : tilesets)
		{
			if(ts->getImagePath().empty())
			{
				std::cout << "This example does not support Collection of Images tilesets\n";
				std::cout << "Chunks using " + ts->getName() + " will not be created\n";
				continue;
			}

			// std::cout << "loading tile texture " << ts->getImagePath() << "\n";
			m_chunkArrays.emplace_back(std::make_unique<ChunkArray>(load_texture(ts->getImagePath()), *ts));
		}
		int xPos = static_cast<int>(position.x / static_cast<float>(tileSize.x));
		int yPos = static_cast<int>(position.y / static_cast<float>(tileSize.y));
		for (auto y = yPos; y < yPos + tileCount.y; ++y)
		{
			for (auto x = xPos; x < xPos + tileCount.x; ++x)
			{
				auto idx = (y * rowSize + x);
				m_chunkTileIDs.emplace_back(tileIDs[static_cast<std::size_t>(idx)]);
				m_chunkColors.emplace_back(vertColour);
			}
		}
		generateTiles(true);
	}

	~Chunk() = default;
	Chunk(const Chunk&) = delete;
	Chunk& operator = (const Chunk&) = delete;

	void generateTiles(bool registerAnimation = false)
	{
		if (registerAnimation)
		{
			m_activeAnimations.clear();
		}
		for (const auto& ca : m_chunkArrays)
		{
			std::size_t idx = 0;
			int xPos = static_cast<int>(getPosition().x / static_cast<float>(mapTileSize.x));
			int yPos = static_cast<int>(getPosition().y / static_cast<float>(mapTileSize.y));
			for (auto y = yPos; y < yPos + chunkTileCount.y; ++y)
			{
				for (auto x = xPos; x < xPos + chunkTileCount.x; ++x)
				{
					if (idx < m_chunkTileIDs.size() && m_chunkTileIDs[idx].ID >= ca->m_firstGID
						&& m_chunkTileIDs[idx].ID <= ca->m_lastGID)
					{
						if (registerAnimation && m_animTiles.find(m_chunkTileIDs[idx].ID) != m_animTiles.end())
						{
							AnimationState as;
							as.animTile = m_animTiles[m_chunkTileIDs[idx].ID];
							as.startTime = 0.0f;
							as.tileCords = glm::ivec2(x,y);
							m_activeAnimations.push_back(as);
						}

						glm::vec2 tileOffset
						{
							x * mapTileSize.x,
							y * mapTileSize.y + mapTileSize.y - static_cast<int>(ca->tileSetSize.y)
						};

						auto idIndex = m_chunkTileIDs[idx].ID - ca->m_firstGID;
						glm::vec2 tileIndex
						{
							(idIndex % static_cast<unsigned int>(ca->tsTileCount.x)) * ca->tileSetSize.x,
							(idIndex / static_cast<unsigned int>(ca->tsTileCount.x)) * ca->tileSetSize.y
						};
						RenderQuad tile =
						{
							::render::Vertex2
							{
								tileOffset - getPosition(),
								m_chunkColors[idx],
								transform_tile(tileIndex, ca->texSize)
							},
							::render::Vertex2
							{
								tileOffset - getPosition() + glm::vec2(ca->tileSetSize.x, 0.f),
								m_chunkColors[idx],
								transform_tile(tileIndex + glm::vec2(ca->tileSetSize.x, 0.f), ca->texSize)
							},
							::render::Vertex2
							{
								tileOffset - getPosition() + glm::vec2(ca->tileSetSize.x, ca->tileSetSize.y),
								m_chunkColors[idx],
								transform_tile(tileIndex + glm::vec2(ca->tileSetSize.x, ca->tileSetSize.y), ca->texSize)
							},
							::render::Vertex2
							{
								tileOffset - getPosition() + glm::vec2(0.f, ca->tileSetSize.y),
								m_chunkColors[idx],
								transform_tile(tileIndex + glm::vec2(0.f, ca->tileSetSize.y), ca->texSize)
							}
						};
						doFlips
						(
							m_chunkTileIDs[idx].flipFlags,
							&tile[0].texturecoord,
							&tile[1].texturecoord,
							&tile[2].texturecoord,
							&tile[3].texturecoord
						);
						ca->addTile(tile);
					}
					idx++;
				}
			}
		}
	}

	void setTile(int x, int y, tmx::TileLayer::Tile tile, bool refresh)
	{
		m_chunkTileIDs[calcIndexFrom(x,y)] = tile;
		
		if (refresh)
		{
			for (const auto& ca : m_chunkArrays)
			{
				ca->reset();
			}
			generateTiles();
		}
	}

	std::size_t calcIndexFrom(int x, int y) const
	{
		return static_cast<std::size_t>(x + y * chunkTileCount.x);
	}

	bool empty() const { return m_chunkArrays.empty(); }

	void draw(render::SpriteBatch& batch) const
	{
		// states.transform *= getTransform();
		for (const auto& a : m_chunkArrays)
		{
			a->draw(batch);
		}
	}
};



struct MapLayer
{
	glm::ivec2 m_chunkSize;
	glm::ivec2 m_chunkCount;
	glm::ivec2 m_MapTileSize;   // general Tilesize of Map
	Rectf m_globalBounds = Rectf{0.0f, 0.0f};

	std::vector<std::unique_ptr<Chunk>> m_chunks;
	mutable std::vector<Chunk*> m_visibleChunks;

	MapLayer(const tmx::Map& map, std::size_t idx)
	{
		const auto& layers = map.getLayers();

		// increasing design_chunk_size by 4; fixes render problems when mapsize != chunksize
		// glm::vec2 design_chunk_size = glm::vec2(1024.f, 1024.f);
		glm::ivec2 design_chunk_size = glm::vec2(512, 512);
		
		//round the chunk size to the nearest tile
		// const auto tileSize = map.getTileSize();
		glm::ivec2 tileSize(map.getTileSize().x, map.getTileSize().y);
		m_chunkSize.x = floor_to_int(static_cast<float>(design_chunk_size.x) / static_cast<float>(tileSize.x)) * static_cast<int>(tileSize.x);
		m_chunkSize.y = floor_to_int(static_cast<float>(design_chunk_size.y) / static_cast<float>(tileSize.y)) * static_cast<int>(tileSize.y);
		m_MapTileSize.x = static_cast<int>(map.getTileSize().x);
		m_MapTileSize.y = static_cast<int>(map.getTileSize().y);
		const auto& layer = layers[idx]->getLayerAs<tmx::TileLayer>();
		
		// create chunks
		{
			//look up all the tile sets and load the textures
			const auto& tileSets = map.getTilesets();
			const auto& layerIDs = layer.getTiles();
			std::uint32_t maxID = std::numeric_limits<std::uint32_t>::max();
			std::vector<const tmx::Tileset*> usedTileSets;

			for (auto i = tileSets.rbegin(); i != tileSets.rend(); ++i)
			{
				for (const auto& tile : layerIDs)
				{
					if (tile.ID >= i->getFirstGID() && tile.ID < maxID)
					{
						usedTileSets.push_back(&(*i));
						break;
					}
				}
				maxID = i->getFirstGID();
			}
			
			for (const auto& ts : usedTileSets)
			{
				if (ts->hasTransparency())
				{
					throw "unable to support transparent textures";
					// auto transparency = ts->getTransparencyColour();
					// img.createMaskFromColor({ transparency.r, transparency.g, transparency.b, transparency.a });
				}
			}

			//calculate the number of chunks in the layer
			//and create each one
			const auto bounds = map.getBounds();
			m_chunkCount.x = static_cast<int>(std::ceil(bounds.width / static_cast<float>(m_chunkSize.x)));
			m_chunkCount.y = static_cast<int>(std::ceil(bounds.height / static_cast<float>(m_chunkSize.y)));

			for (int y = 0; y < m_chunkCount.y; ++y)
			{
				glm::vec2 tileCount
				{
					static_cast<float>(m_chunkSize.x) / static_cast<float>(tileSize.x),
					static_cast<float>(m_chunkSize.y) / static_cast<float>(tileSize.y)
				};
				for (int x = 0; x < m_chunkCount.x; ++x)
				{
					// calculate size of each Chunk (clip against map)
					if( static_cast<float>( (x + 1) * m_chunkSize.x) > bounds.width)
					{
						tileCount.x = (bounds.width - static_cast<float>(x * m_chunkSize.x)) / static_cast<float>(map.getTileSize().x);
					}
					if( static_cast<float>( (y + 1) * m_chunkSize.y) > bounds.height)
					{
						tileCount.y = (bounds.height - static_cast<float>(y * m_chunkSize.y)) / static_cast<float>(map.getTileSize().y);
					}
					m_chunks.emplace_back(std::make_unique<Chunk>(
						layer,
						usedTileSets,
						glm::vec2(x * m_chunkSize.x, y * m_chunkSize.y),
						tileCount,
						tileSize,
						map.getTileCount().x,
						map.getAnimatedTiles()
					));
				}
			}
		}
		//

		auto mapSize = map.getBounds();
		m_globalBounds = Rectf{mapSize.width, mapSize.height};
	}

	~MapLayer() = default;
	MapLayer(const MapLayer&) = delete;
	MapLayer& operator = (const MapLayer&) = delete;

	MapLayer(MapLayer&&) = default;
	MapLayer& operator = (MapLayer&&) = default;

	void update(float elapsed) 
	{
		auto setTile = [this](int x, int y, tmx::TileLayer::Tile tile)
		{
			bool refresh = true;
			int chunkX = floor_to_int(static_cast<float>(x * m_MapTileSize.x) / static_cast<float>(m_chunkSize.x));
			int chunkY = floor_to_int(static_cast<float>(y * m_MapTileSize.y) / static_cast<float>(m_chunkSize.y));
			const int chtax = x * m_MapTileSize.x - chunkX * m_chunkSize.x;
			const int chtay = y * m_MapTileSize.y - chunkY * m_chunkSize.y;
			glm::ivec2 chunkLocale =
			{
				static_cast<int>( static_cast<float>(chtax) / static_cast<float>(m_MapTileSize.x)),
				static_cast<int>( static_cast<float>(chtay) / static_cast<float>(m_MapTileSize.y))
			};
			const auto index = chunkX + chunkY * m_chunkCount.x;
			const auto& selectedChunk = m_chunks[static_cast<size_t>(index)];
			// const auto& selectedChunk = getChunkAndTransform(x, y, chunkLocale);
			selectedChunk->setTile(chunkLocale.x, chunkLocale.y, tile, refresh);
		};

		for (auto& c : m_visibleChunks) 
		{
			for (AnimationState& as : c->m_activeAnimations) 
			{
				as.currentTime += elapsed;

				tmx::TileLayer::Tile tile;
				float animTime = 0;
				auto x = as.animTile.animation.frames.begin();
				while (animTime < as.currentTime)
				{
					if (x == as.animTile.animation.frames.end()) 
					{
						x = as.animTile.animation.frames.begin();
						as.currentTime -= animTime;
						animTime = 0;
					}

					tile.ID = x->tileID;
					animTime += static_cast<float>(x->duration)/1000.0f;
					x++;
				}

				setTile(as.tileCords.x, as.tileCords.y, tile);
			}
		}
	}

	void draw(render::SpriteBatch& batch, const Rectf& view) const
	{
		// update visibility:
		// calc view coverage and draw nearest chunks
		{
			const int left = static_cast<int>(view.left);
			const int bottom = static_cast<int>(view.bottom);
			const int right = static_cast<int>(view.right);
			const int top = static_cast<int>(view.top);

			std::vector<Chunk*> visible;
			for (auto y = bottom; y < top; ++y)
			{
				for (auto x = left; x < right; ++x)
				{
					std::size_t idx = static_cast<std::size_t>(y * m_chunkCount.x + x);
					if (idx < m_chunks.size() && !m_chunks[idx]->empty())
					{
						visible.push_back(m_chunks[idx].get());
					}
				}
			}

			std::swap(m_visibleChunks, visible);
		}


		for (const auto& c : m_visibleChunks)
		{
			c->draw(batch);
			// rt.draw(*c, states);
		}
	}
};


std::optional<MapLayer> make_layer(const tmx::Map& map, std::size_t idx)
{
	const auto& layers = map.getLayers();
	if (map.getOrientation() == tmx::Orientation::Orthogonal &&
		idx < layers.size() && layers[idx]->getType() == tmx::Layer::Type::Tile)
	{
		return MapLayer{map, idx};
	}
	else
	{
		return {};
	}
}


struct MapImpl
{
	std::vector<MapLayer> layers;
};


Map::Map()
	: impl(std::make_unique<MapImpl>())
{
}

Map::~Map() = default;

void Map::load_from_map(const tmx::Map& map)
{
	impl->layers.clear();

	const auto& layers = map.getLayers();
	for(std::size_t index=0; index < layers.size(); index+=1)
	{
		auto loaded = make_layer(map, index);
		if(loaded)
		{
			impl->layers.emplace_back(std::move(*loaded));
		}
	}
}

void Map::update(float dt)
{
	for(auto& layer: impl->layers)
	{
		layer.update(dt);
	}
}

void Map::render(render::SpriteBatch& batch, const Rectf& view)
{
	for(auto& layer: impl->layers)
	{
		layer.draw(batch, view);
	}
}

