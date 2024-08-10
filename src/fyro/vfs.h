#pragma once

struct Physfs
{
	Physfs(const Physfs &) = delete;
	Physfs(Physfs &&) = delete;
	Physfs operator=(const Physfs &) = delete;
	Physfs operator=(Physfs &&) = delete;

	Physfs(const std::string &path);
	~Physfs();
	void setup(const std::string &root);
	void setup_with_default_root();
};

std::vector<char> read_file_to_bytes(const std::string &path);

std::string read_file_to_string(const std::string &path);
std::optional<std::string> read_file_to_string_or_none(const std::string &path);
