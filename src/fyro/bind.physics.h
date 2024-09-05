#pragma once

#include "fyro/collision2.h"
#include "fyro/tiles.h"

namespace lox
{
struct Lox;
}

struct ScriptLevelData
{
	fyro::Level level;
	Map tiles;

	std::map<std::string, std::shared_ptr<lox::Callable>> from_tileset;
};

struct ScriptLevel
{
	std::shared_ptr<ScriptLevelData> data;

	ScriptLevel();
	void load_tmx(const std::string& path);
	void add_actor(std::shared_ptr<lox::Instance> x);
	void add_solid(std::shared_ptr<lox::Instance> x);
};

namespace bind
{

void bind_phys_actor(lox::Lox* lox);
void bind_phys_solid(lox::Lox* lox);
void bind_phys_level(lox::Lox* lox);

}  //  namespace bind
