module;
// This module exists solely for the purpose
// of exporting the Vulkan version macros in a C++ way.
// Needed for VK_MAKE_VERSION macros
#include <vulkan/vulkan.h>
export module vulkan_version;

import std;

namespace tektonik::vulkan
{
export constexpr std::uint32_t kVulkanApiVersion_1_4 = VK_API_VERSION_1_4;

export constexpr std::uint32_t MakeApiVersion(std::uint32_t variant, std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept
{
    return VK_MAKE_API_VERSION(variant, major, minor, patch);
}

export constexpr std::uint32_t MakeVersion(std::uint32_t major, std::uint32_t minor, std::uint32_t patch) noexcept
{
    return VK_MAKE_VERSION(major, minor, patch);
}
}  // namespace tektonik::vulkan
