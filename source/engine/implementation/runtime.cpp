module;
#include "sdl-wrapper.hpp"
module runtime;

import test;
import logger;
import singleton;

namespace tektonik
{

void Runtime::Test()
{
    Singleton<Logger>::Init();
    test::RunAll();
}

void Runtime::OpenDemoWindow()
{
    Singleton<Logger>::Init();
    Singleton<Logger>::Get().Log("Opening a demo SDL window...");

    if (!RunDemoSdlApp())
        Singleton<Logger>::Get().Log<LogLevel::Error>("There is a problem with SDL");
}

}  // namespace tektonik
