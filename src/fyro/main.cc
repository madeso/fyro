#include "fyro/io.h"
#include "fyro/log.h"
#include "fyro/exception.h"
#include "fyro/vfs.h"
#include "fyro/gamedata.h"
#include "fyro/game.h"

int run(int argc, char** argv)
{
	auto physfs = Physfs(argv[0]);
	bool call_imgui = false;
	std::optional<std::string> folder_arg;

	for (int index = 1; index < argc; index += 1)
	{
		const std::string cmd = argv[index];
		if (cmd == "--imgui")
		{
			call_imgui = true;
		}
		else
		{
			if (folder_arg)
			{
				LOG_WARNING("Folder already specified as {0}: {1}", *folder_arg, cmd);
				return -1;
			}
			else
			{
				folder_arg = cmd;
			}
		}
	}

	if (folder_arg)
	{
		physfs.setup(cannonical_folder(*folder_arg));
	}
	else
	{
		physfs.setup_with_default_root();
	}

	const auto data = load_game_data_or_default("main.json");

	return run_game(
		data.title,
		glm::ivec2{data.width, data.height},
		call_imgui,
		[]()
		{
			auto game = std::make_shared<ExampleGame>();
			game->run_main();
			return game;
		}
	);
}

int main(int argc, char** argv)
{
	try
	{
		return run(argc, argv);
	}
	catch (...)
	{
		auto x = collect_exception();
		for (const auto& e: x.errors)
		{
			LOG_ERROR("- {0}", e);
		}
		return -1;
	}
}
