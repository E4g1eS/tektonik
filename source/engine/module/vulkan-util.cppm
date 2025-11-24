module;
#include "common-defines.hpp"
#include "sdl-wrapper.hpp"
export module vulkan_util;

import util;
import singleton;
import logger;
import concepts;
import vulkan_hpp;
import glm;

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

export class RaiiWindowWrapper
{
  public:
    struct CreateInfo
    {
        std::string title = "Untitled window";
        glm::ivec2 size = {800, 600};
        SDL_WindowFlags flags = 0;
    };

    RaiiWindowWrapper() noexcept = default;
    RaiiWindowWrapper(const CreateInfo& createInfo)
    {
        window = SDL_CreateWindow(createInfo.title.c_str(), createInfo.size.x, createInfo.size.y, createInfo.flags | SDL_WINDOW_VULKAN);
        if (!window)
            throw std::runtime_error("Could not create SDL window.");
    }
    ~RaiiWindowWrapper()
    {
        if (window)
            SDL_DestroyWindow(window);
    }

    RaiiWindowWrapper(RaiiWindowWrapper&& other) : window(std::exchange(other.window, nullptr)) {}
    RaiiWindowWrapper& operator=(RaiiWindowWrapper&& other)
    {
        if (this != &other)
            window = std::exchange(other.window, nullptr);
        return *this;
    }

    RaiiWindowWrapper(const RaiiWindowWrapper& other) = delete;
    RaiiWindowWrapper& operator=(const RaiiWindowWrapper& other) = delete;

    SDL_Window* operator*() { return window; }

  private:
    SDL_Window* window;
};

/// Vulkan KHR surface wrapper with SDL constructor and destructor.
export class RaiiSurfaceWrapper
{
  public:
    RaiiSurfaceWrapper() noexcept = default;
    RaiiSurfaceWrapper(const vk::raii::Instance& instance, SDL_Window* window) : instance(*instance)
    {
        VkSurfaceKHR cSurface;
        if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &cSurface))
            throw std::runtime_error("Could not create a Vulkan surface.");

        surface = cSurface;
    }
    RaiiSurfaceWrapper(const vk::raii::Instance& instance, RaiiWindowWrapper& windowWrapper) : RaiiSurfaceWrapper(instance, *windowWrapper) {}
    ~RaiiSurfaceWrapper()
    {
        if (instance && surface)
            SDL_Vulkan_DestroySurface(instance, surface, nullptr);
    }

    RaiiSurfaceWrapper(const RaiiSurfaceWrapper&) = delete;
    RaiiSurfaceWrapper(RaiiSurfaceWrapper&& other) noexcept = default;
    RaiiSurfaceWrapper& operator=(const RaiiSurfaceWrapper&) = delete;
    RaiiSurfaceWrapper& operator=(RaiiSurfaceWrapper&& other) noexcept = default;

    auto& operator*(this auto&& self) { return self.surface; }

  private:
    vk::Instance instance{nullptr};
    vk::SurfaceKHR surface{nullptr};
};

}  // namespace tektonik::vulkan::util
