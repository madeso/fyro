#include "fyro/vfs.h"

#include "fyro/types.h"
#include "fyro/log.h"
#include "fyro/exception.h"
#include "fyro/io.h"

#include "physfs.h"

std::string get_physfs_error()
{
	const auto code = PHYSFS_getLastErrorCode();
	return PHYSFS_getErrorByCode(code);
}

std::vector<std::string> physfs_files_in_dir(const std::string &dir)
{
	constexpr const auto callback = [](void *data, const char *origdir, const char *fname) -> PHYSFS_EnumerateCallbackResult
	{
		auto *ret = static_cast<std::vector<std::string> *>(data);
		ret->emplace_back(std::string(origdir) + fname);
		return PHYSFS_ENUM_OK;
	};

	std::vector<std::string> r;
	auto ok = PHYSFS_enumerate(dir.c_str(), callback, &r);

	if (ok == 0)
	{
		LOG_WARNING("failed to enumerate {0}", dir);
	}

	return r;
}

std::optional<std::vector<char>> read_file_to_bytes_or_none(const std::string &path)
{
	auto *file = PHYSFS_openRead(path.c_str());
	if (file == nullptr)
	{
		return std::nullopt;
	}

	std::vector<char> ret;
	while (PHYSFS_eof(file) == 0)
	{
		constexpr u64 buffer_size = 1024;
		char buffer[buffer_size] = {
			0,
		};
		const auto read = PHYSFS_readBytes(file, buffer, buffer_size);
		if (read > 0)
		{
			ret.insert(ret.end(), buffer, buffer + read);
		}
	}

	ret.emplace_back('\0');

	PHYSFS_close(file);
	return ret;
}

Exception physfs_exception(const std::string &message)
{
	const std::string error = get_physfs_error();
	return Exception{{message + error}};
}

Exception missing_file_exception(const std::string &path)
{
	// todo(Gustav): split path and find files in dir
	return physfs_exception("unable to open file")
		.append("found files")
		.append(physfs_files_in_dir(get_dir_from_file(path)));
}

std::vector<char> read_file_to_bytes(const std::string &path)
{
	if (auto ret = read_file_to_bytes_or_none(path))
	{
		return *ret;
	}
	else
	{
		throw missing_file_exception(path);
	}
}

std::optional<std::string> read_file_to_string_or_none(const std::string &path)
{
	if (auto ret = read_file_to_bytes_or_none(path))
	{
		ret->emplace_back('\0');
		return ret->data();
	}
	else
	{
		return std::nullopt;
	}
}

std::string read_file_to_string(const std::string &path)
{
	if (auto ret = read_file_to_string_or_none(path))
	{
		return *ret;
	}
	else
	{
		throw missing_file_exception(path);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Physfs

Physfs::Physfs(const std::string &path)
{
	const auto ok = PHYSFS_init(path.c_str());
	if (ok == 0)
	{
		throw physfs_exception("unable to init");
	}
}

Physfs::~Physfs()
{
	const auto ok = PHYSFS_deinit();
	if (ok == 0)
	{
		const std::string error = get_physfs_error();
		LOG_ERROR("Unable to shut down physfs: {0}", error);
	}
}

void Physfs::setup(const std::string &root)
{
	LOG_INFO("Sugggested root is: {0}", root);
	PHYSFS_mount(root.c_str(), "/", 1);
}

void Physfs::setup_with_default_root()
{
	setup(PHYSFS_getBaseDir());
}
