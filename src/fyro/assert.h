#pragma once



#ifdef _MSC_VER
	#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define ASSERT(x) \
	do \
	{ \
		if (x) \
		{ \
		} \
		else \
		{ \
			::on_assert_failure(#x, __PRETTY_FUNCTION__, __FILE__, __LINE__); \
		} \
	} while (false)
#define DIE(reason) ::on_assert_failure(reason, __PRETTY_FUNCTION__, __FILE__, __LINE__)

void on_assert_failure(
	std::string_view function, std::string_view reason, std::string_view file, int line
);
void enable_exception_on_assert();
