#pragma once

#include <string_view>

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define ASSERT(x) do { if(x) {} else { ::on_assert_failure(#x, __PRETTY_FUNCTION__, __FILE__, __LINE__); } } while(false)

void on_assert_failure(std::string_view function, std::string_view reason, std::string_view file, int line);
void enable_exception_on_assert();