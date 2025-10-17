module;
#include "sdl-wrapper.hpp"
module runtime;

import test;
import logger;
import singleton;

namespace tektonik
{

void Runtime::Test() const
{
    test::RunAll();
}

}  // namespace tektonik
