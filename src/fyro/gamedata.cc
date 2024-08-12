#include "fyro/gamedata.h"

#include "fyro/exception.h"
#include "fyro/vfs.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::optional<json> load_json_or_none(const std::string& path)
{
	if (const auto src = read_file_to_string_or_none(path))
	{
		auto parsed = json::parse(*src);
		return parsed;	// assigning to a variable and then returning makes gcc happy
	}
	else
	{
		return std::nullopt;
	}
}

GameData load_game_data_or_default(const std::string& path)
{
	try
	{
		if (const auto loaded_data = load_json_or_none(path); loaded_data)
		{
			const auto& data = *loaded_data;

			auto r = GameData{};
			r.title = data["title"].get<std::string>();
			r.width = data["width"].get<int>();
			r.height = data["height"].get<int>();
			return r;
		}
		else
		{
			auto r = GameData{};
			r.title = "Example";
			r.width = 800;
			r.height = 600;
			return r;
		}
	}
	catch (...)
	{
		auto x = collect_exception();
		x.append("while reading " + path);
		throw x;
	}
}
