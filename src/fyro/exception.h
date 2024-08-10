#pragma once

struct Exception
{
	std::vector<std::string> errors;

	Exception append(const std::string &str);

	Exception append(const std::vector<std::string> &li);
};

Exception collect_exception();
