#include "fyro/assert.h"

#include <cstdlib>


#include <fmt/printf.h>

namespace
{
    bool& is_exception_enabled()
    {
        static bool enabled = false;
        return enabled;
    }
}


void on_assert_failure(std::string_view function, std::string_view reason, std::string_view file, int line)
{
    const auto text = fmt::format("{}({}): Assertion failed in {}: {}", file, line, function, reason);
    
    if(is_exception_enabled())
    {
        throw text;
    }
    else
    {
        fmt::print("{}", text);
        fflush(stdout);
        std::abort();
    }
}


void enable_exception_on_assert()
{
    is_exception_enabled() = true;
}
