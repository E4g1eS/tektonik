module;
#include "common-defines.hpp"
#include "sdl-wrapper.hpp"
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

/// Vulkan KHR surface wrapper with SDL constructor and destructor.
export class SdlRaiiSurfaceWrapper
{
  public:
    SdlRaiiSurfaceWrapper() noexcept = default;
    SdlRaiiSurfaceWrapper(const vk::raii::Instance& instance, SDL_Window* window);
    ~SdlRaiiSurfaceWrapper();

    SdlRaiiSurfaceWrapper(const SdlRaiiSurfaceWrapper&) = delete;
    SdlRaiiSurfaceWrapper(SdlRaiiSurfaceWrapper&& other) noexcept = default;
    SdlRaiiSurfaceWrapper& operator=(const SdlRaiiSurfaceWrapper&) = delete;
    SdlRaiiSurfaceWrapper& operator=(SdlRaiiSurfaceWrapper&& other) noexcept = default;

    vk::SurfaceKHR& operator*() { return surface; }

  private:
    vk::Instance instance{nullptr};
    vk::SurfaceKHR surface{nullptr};
};

SdlRaiiSurfaceWrapper::SdlRaiiSurfaceWrapper(const vk::raii::Instance& instance, SDL_Window* window) : instance(instance)
{
    VkSurfaceKHR cSurface;
    if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &cSurface))
        throw std::runtime_error("Could not create a Vulkan surface.");

    surface = cSurface;
}

SdlRaiiSurfaceWrapper::~SdlRaiiSurfaceWrapper()
{
    if (instance && surface)
        SDL_Vulkan_DestroySurface(instance, surface, nullptr);
}


}  // namespace tektonik::vulkan::util
