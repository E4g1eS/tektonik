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

void Runtime::OpenDemoWindow() const
{
    Singleton<Logger>::Get().Log("Opening a demo SDL window...");

    if (!RunDemoSdlApp())
        Singleton<Logger>::Get().Log<LogLevel::Error>("There is a problem with SDL");
}

}  // namespace tektonik
