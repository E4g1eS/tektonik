module;
#include "imgui-wrapper.hpp"
#include "sdl-wrapper.hpp"
#include "std.hpp"
module config_renderer;

import vulkan;

namespace tektonik::config
{

void Renderer::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();



}

}  // namespace tektonik::config
