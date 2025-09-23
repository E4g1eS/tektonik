module;
module runtime;

import test;

namespace runtime
{

void Runtime::Run()
{
    if (!test::RunAll())
        return;

    auto app = app::App();
    app.Init();
}
}  // namespace runtime
