module;
#include "std.hpp"
export module vulkan_util;

import util;
import singleton;
import logger;
import concepts;
import vulkan_hpp;

namespace tektonik::vulkan::util
{

export bool AreInstanceLayersSupported(const vk::raii::Context& context, const concepts::RangeOfCastableTo<std::string_view> auto& wanted)
{
    auto availables = context.enumerateInstanceLayerProperties();
    auto availableNames = std::views::transform(availables, [](const auto& layer) { return layer.layerName; });

    auto unavailables = tektonik::util::string::HasAll(availableNames, wanted);

    for (const auto& unavailable : unavailables)
        Singleton<Logger>::Get().Log<LogLevel::Warning>(std::format("Wanted layer: '{}' is not available.", unavailable));

    return unavailables.empty();
}

export bool AreInstanceExtensionsSupported(
    const vk::raii::Context& context,
    const concepts::RangeOfCastableTo<std::string_view> auto& wanted,
    const vk::Optional<const std::string>& layer = {nullptr})
{
    auto availables = context.enumerateInstanceExtensionProperties(layer);
    auto availableNames = std::views::transform(availables, [](const auto& extension) { return extension.extensionName; });

    auto unavailables = tektonik::util::string::HasAll(availableNames, wanted);

    for (const auto& unavailable : unavailables)
        Singleton<Logger>::Get().Log<LogLevel::Warning>(std::format("Wanted layer: '{}' is not available.", unavailable));

    return unavailables.empty();
}

export bool AreDeviceExtensionsSupported(
    const vk::raii::PhysicalDevice& physicalDevice,
    const concepts::RangeOfCastableTo<std::string_view> auto& wanted,
    const vk::Optional<const std::string>& layer = {nullptr})
{
    auto availables = physicalDevice.enumerateDeviceExtensionProperties(layer);
    auto availableNames = std::views::transform(availables, [](const auto& extension) { return extension.extensionName; });

    auto unavailables = tektonik::util::string::HasAll(availableNames, wanted);

    for (const auto& unavailable : unavailables)
        Singleton<Logger>::Get().Log<LogLevel::Warning>(std::format("Wanted layer: '{}' is not available.", unavailable));

    return unavailables.empty();
}

}  // namespace tektonik::vulkan::util
