module;
#include "std.hpp"
export module runtime;

import app;
import logger;
import singleton;
import config;
import config_renderer;
import sdl_runtime;

export namespace tektonik
{

// All engine code must run while a Runtime is created.
class Runtime
{
  public:
    struct RunOptions
    {
        int argc = 0;
        char** argv = nullptr;
    };

    Runtime(const RunOptions& runOptions = RunOptions{}) : runOptions(runOptions) {}
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    void Init();

    // Runs tests.
    void Test() const;

  private:
    const RunOptions runOptions;
    Singleton<Logger> logger;
    Singleton<config::Manager> configManager;
    SdlRuntime sdlRuntime;
    config::Renderer configRenderer;
    std::jthread configRendererThread;
};

}  // namespace tektonik
