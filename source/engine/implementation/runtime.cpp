module;
module runtime;

import test;

namespace runtime
{

void Runtime::Run(const RunOptions& runOptions)
{
    if (runOptions.runTests && !test::RunAll())
        return;

    auto app = app::App();
    app.Init();
}
}  // namespace runtime
