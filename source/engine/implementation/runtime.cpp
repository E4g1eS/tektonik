module;
#include "sdl-wrapper.hpp"
#include "common-defines.hpp"
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

    SDL_Event event;

    bool running = true;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
            {
                running = false;
                break;
            }

            configRenderer.HandleEvent(event);
        }

        configRenderer.Tick();
    }
}

void Runtime::Test() const
{
    test::RunAll();
}

}  // namespace tektonik
