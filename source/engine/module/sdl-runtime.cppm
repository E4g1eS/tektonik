module;
#include "sdl-wrapper.hpp"
#include "std.hpp"
export module sdl_runtime;

import singleton;
import logger;

namespace tektonik
{

export class SdlRuntime
{
  public:
    SdlRuntime()
    {
        Singleton<Logger>::Get().Log("Initializing SDL...");
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            Singleton<Logger>::Get().Log<LogLevel::Error>(std::format("SDL_Init failed with: '{}'", SDL_GetError()));
            throw std::runtime_error("SDL could not be initialized.");
        }
    }
    ~SdlRuntime()
    {
        Singleton<Logger>::Get().Log("Destroying SDL...");
        SDL_Quit();
    }

    SdlRuntime(const SdlRuntime& other) = delete;
    SdlRuntime& operator=(const SdlRuntime& other) = delete;
};

}  // namespace tektonik
