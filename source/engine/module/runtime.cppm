module;
#include "std.hpp"
export module runtime;

import app;

export namespace runtime
{

class Runtime
{
  public:
    struct RunOptions
    {
        bool runTests = true;
    };

    void Run(const RunOptions& runOptions = RunOptions{});

  private:
};

}  // namespace runtime
