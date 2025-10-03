module;
#include "std.hpp"
export module runtime;

import app;
import logger;
import singleton;

export namespace tektonik
{

// Stub for later handling different runtime options.
class Runtime
{
  public:
    struct RunOptions
    {
    };

    Runtime(const RunOptions& runOptions = RunOptions{})
    {
        Singleton<Logger>::Init();
        Singleton<Logger>::Get().Log("Logger initialized.");
    }
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    ~Runtime()
    {
        Singleton<Logger>::Get().Log("Shutting down.");
        Singleton<Logger>::Destroy();
    }

    // Runs tests.
    static void Test();
    static void OpenDemoWindow();

  private:
};

}  // namespace tektonik
