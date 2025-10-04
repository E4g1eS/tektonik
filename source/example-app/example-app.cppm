module;
#include "std.hpp"
export module example_app;

// Example application using the Tektonik engine.

import tektonik;

namespace tk = tektonik;
using S_Logger = tk::Singleton<tk::Logger>;

export class ExampleApp
{
  public:
    ExampleApp(int argc, char** argv)
        : runtime(
              tektonik::Runtime::RunOptions{
                  .argc = argc,
                  .argv = argv,
              })
    {
    }

    void Run()
    {
        runtime.Test();
        runtime.OpenDemoWindow();
    }

  private:
    tektonik::Runtime runtime;
};
