#pragma once

struct GameData
{
	std::string title = "fyro";
	int width = 800;
	int height = 600;
};

GameData load_game_data_or_default(const std::string &path);
