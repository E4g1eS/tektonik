module;
#include "sdl-wrapper.hpp"
#include "std.hpp"
module runtime;

import test;
import logger;
import singleton;
import util;
import concepts;
import config_renderer;

namespace tektonik
{

void Runtime::Init()
{
    auto argMap = util::string::ParseCommandLineArgumentsToMap(runOptions.argc, runOptions.argv);
    Singleton<Logger>::Get().Log(std::format("Loaded command line arguments: {}", argMap));

    configRenderer.Init();
    configRendererThread = std::jthread([this](std::stop_token stopToken) { configRenderer.Loop(stopToken); });

    SDL_Event event;
    while (true)
    {
        SDL_PollEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            break;
    }

    configRendererThread.request_stop();
    ASSUMERT(configRendererThread.joinable());
    configRendererThread.join();

    Singleton<Logger>::Get().Log("Config renderer thread joined.");
}

void Runtime::Test() const
{
    test::RunAll();
}

}  // namespace tektonik
