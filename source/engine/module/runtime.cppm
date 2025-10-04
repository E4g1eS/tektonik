module;
#include "std.hpp"
export module runtime;

import app;
import logger;
import singleton;
import config;

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

    Runtime(const RunOptions& runOptions = RunOptions{})
    {
    }
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    // Runs tests.
    void Test() const;
    void OpenDemoWindow() const;

  private:
    Singleton<Logger> logger;
    Singleton<config::Manager> configManager;
};

}  // namespace tektonik
