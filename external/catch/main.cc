#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "fyro/assert.h"


int main( int argc, char* argv[])
{
    enable_exception_on_assert();
    return Catch::Session().run( argc, argv );
}
