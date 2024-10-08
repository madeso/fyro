#pragma once

void log_info_implementation(fmt::string_view format, fmt::format_args args);

void log_error_implementation(fmt::string_view format, fmt::format_args args);

void log_warning_implementation(fmt::string_view format, fmt::format_args args);

template<typename S, typename... Args>
void log_info(const S& format, Args&&... args)
{
	log_info_implementation(format, fmt::make_format_args(args...));
}

template<typename S, typename... Args>
void log_error(const S& format, Args&&... args)
{
	log_error_implementation(format, fmt::make_format_args(args...));
}

template<typename S, typename... Args>
void log_warning(const S& format, Args&&... args)
{
	log_warning_implementation(format, fmt::make_format_args(args...));
}

// todo(Gustav): fix va args expansion with no arguments

#if 0
	#define LOG_INFO(format, ...) LogInfo(FMT_STRING(format), ##__VA_ARGS__)
	#define LOG_ERROR(format, ...) LogError(FMT_STRING(format), ##__VA_ARGS__)
#else
	#define LOG_INFO(...) ::log_info(__VA_ARGS__)
	#define LOG_ERROR(...) ::log_error(__VA_ARGS__)
	#define LOG_WARNING(...) ::log_warning(__VA_ARGS__)
#endif
