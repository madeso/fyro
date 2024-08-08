#include "fyro/io.h"

std::string get_dir_from_file(const std::string &path)
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
