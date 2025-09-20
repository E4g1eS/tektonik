module;
#include "sdl-wrapper.hpp"
#include "std.hpp"
module app;

namespace app
{
void App::Init()
{
    if (!RunDemoSdlApp())
        std::cout << "There is a problem with SDL" << std::endl;
    std::cout << "App initialized." << std::endl;
}
}  // namespace app
