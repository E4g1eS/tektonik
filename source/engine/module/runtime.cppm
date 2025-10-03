module;
#include "std.hpp"
export module runtime;

import app;

export namespace tektonik
{

// Stub for later handling different runtime options.
class Runtime
{
  public:
    struct RunOptions
    {
    };

    Runtime(const RunOptions& runOptions = RunOptions{}) {}

    // Runs tests.
    static void Test();

  private:
};

}  // namespace runtime
