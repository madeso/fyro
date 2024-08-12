#include "fyro/io.h"

#include <filesystem>

std::string get_dir_from_file(const std::string& path)
{
	if (const auto slash = path.rfind('/'); slash != std::string::npos)
	{
		return path.substr(0, slash - 1);
	}
	else
	{
		return path;
	}
}

std::string cannonical_folder(const std::string& src)
{
	namespace fs = std::filesystem;

	fs::path folder = src;
	folder = fs::canonical(folder);
	const auto ret = folder.string();
	return ret;
}
