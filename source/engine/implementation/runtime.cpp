module;
#include "sdl-wrapper.hpp"
#include "std.hpp"
module runtime;

import test;
import logger;
import singleton;
import util;
import concepts;

namespace tektonik
{

Runtime::Runtime(const RunOptions& runOptions)
{
    auto argMap = util::string::ParseCommandLineArgumentsToMap(runOptions.argc, runOptions.argv);
    Singleton<Logger>::Get().Log(std::format("Loaded command line arguments: {}", argMap));
}

void Runtime::Test() const
{
    test::RunAll();
}

}  // namespace tektonik
