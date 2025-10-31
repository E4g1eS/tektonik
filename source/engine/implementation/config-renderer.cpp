module;
#include "imgui-wrapper.hpp"
#include "sdl-wrapper.hpp"
#include "std.hpp"
#include "vulkan-wrapper.hpp"
module config_renderer;

import singleton;
import util;
import logger;
import vulkan_util;

namespace tektonik::config
{

void CheckImGuiVulkanResult(VkResult result)
{
    if (result == VK_SUCCESS)
        return;

    std::string errorStr = vk::to_string(vk::Result(result));
    Singleton<Logger>::Get().Log<LogLevel::Error>(std::format("ImGUI Vulkan ran into error: {}", errorStr));
}

uint32_t GetGraphicsQueueFamily(const vk::raii::PhysicalDevice physicalDevice)
{
    std::vector<vk::QueueFamilyProperties> queueFamiliesProperties = physicalDevice.getQueueFamilyProperties();
    for (const auto& [familyIndex, queueFamilyProperties] : std::views::enumerate(queueFamiliesProperties))
        if (queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
            return familyIndex;

    throw std::runtime_error("Physical device does not have a graphics queue family.");
}

vk::raii::RenderPass CreateRenderPass(const vk::raii::Device& device)
{
    vk::AttachmentDescription colorAttachment{
        .format = vk::Format::eR8G8B8A8Unorm,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,  // Clear to black
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,  // Dont care about stencil buffer
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,  // So the image is ready to be presented
    };

    vk::AttachmentReference colorAttachmentReference{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal,
    };

    vk::SubpassDescription subpass{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pDepthStencilAttachment = nullptr,
    };

    vk::SubpassDependency subpassDependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
    };

    vk::RenderPassCreateInfo createInfo{
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &subpassDependency,
    };

    return device.createRenderPass(createInfo);
}

vk::raii::Instance CreateInstance(const vk::raii::Context& context)
{
    vk::ApplicationInfo applicationInfo{
        .pApplicationName = "Config Renderer",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };

    uint32_t extensionCount = 0;
    auto extensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

    if (!vulkan::util::AreInstanceExtensionsSupported(context, std::span(extensions, extensionCount)))
        throw std::runtime_error("Necessary extensions are not supported");

    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const bool validationSupported = vulkan::util::AreInstanceLayersSupported(context, validationLayers);

    return context.createInstance(
        vk::InstanceCreateInfo{
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = validationSupported ? static_cast<uint32_t>(validationLayers.size()) : 0,
            .ppEnabledLayerNames = validationSupported ? validationLayers.data() : nullptr,
            .enabledExtensionCount = extensionCount,
            .ppEnabledExtensionNames = extensions,
        });
}

bool IsSuitable(const vk::raii::PhysicalDevice& physicalDevice)
{
    // TODO
    return true;
}

vk::raii::PhysicalDevice ChoosePhysicalDevice(const vk::raii::Instance& instance)
{
    std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.empty())
        throw std::runtime_error("There are no Vulkan physical devices.");

    auto foundDevice =
        std::ranges::find_if(physicalDevices, [](const vk::raii::PhysicalDevice& physicalDevice) { return IsSuitable(physicalDevice); });
    if (foundDevice == physicalDevices.end())
        throw std::runtime_error("There are no suitable Vulkan physical devices.");

    return *foundDevice;
}

vk::raii::Device CreateDevice(const vk::raii::PhysicalDevice& physicalDevice, uint32_t queueFamily)
{
    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo queueCreateInfo{
        .queueFamilyIndex = queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    const std::vector<const char*> deviceExtensions = {"VK_KHR_swapchain"};

    return physicalDevice.createDevice(
        vk::DeviceCreateInfo{
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
            .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
        });
}

vk::SurfaceKHR CreateSurface(const vk::raii::Instance& instance, SDL_Window* window)
{
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &surface))
        throw std::runtime_error("Could not create a Vulkan surface.");

    return vk::SurfaceKHR(surface);
}

vk::raii::SwapchainKHR CreateSwapchain(
    const vk::raii::Device& device,
    const vk::SurfaceKHR& surface,
    const vk::Extent2D& windowSize,
    const uint32_t queueFamily)
{
    return device.createSwapchainKHR(
        vk::SwapchainCreateInfoKHR{
            .surface = surface,
            .minImageCount = 2,
            .imageFormat = vk::Format::eR8G8B8A8Unorm,
            .imageExtent = windowSize,
            .imageArrayLayers = 1,
            .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
            .imageSharingMode = vk::SharingMode::eExclusive,
            .queueFamilyIndexCount = {},     // only used with concurrent sharing mode
            .pQueueFamilyIndices = nullptr,  // only used with concurrent sharing mode
            .preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity,
            .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
            .presentMode = vk::PresentModeKHR::eFifo,  // only one required to be supported
            .clipped = true,
        });
}

std::vector<vk::raii::ImageView> CreateSwapchainImageViews(const vk::raii::Device& device, const std::vector<vk::Image>& swapchainImages)
{
    std::vector<vk::raii::ImageView> swapchainImageViews;
    swapchainImageViews.reserve(swapchainImages.size());
    for (const auto& image : swapchainImages)
    {
        vk::ImageViewCreateInfo createInfo{
            .image = image,
            .viewType = vk::ImageViewType::e2D,
            .format = vk::Format::eR8G8B8A8Unorm,
            .components =
                vk::ComponentMapping{
                                     .r = vk::ComponentSwizzle::eIdentity,
                                     .g = vk::ComponentSwizzle::eIdentity,
                                     .b = vk::ComponentSwizzle::eIdentity,
                                     .a = vk::ComponentSwizzle::eIdentity,
                                     },
            .subresourceRange =
                vk::ImageSubresourceRange{
                                     .aspectMask = vk::ImageAspectFlagBits::eColor,
                                     .baseMipLevel = 0,
                                     .levelCount = 1,
                                     .baseArrayLayer = 0,
                                     .layerCount = 1,
                                     },
        };
        swapchainImageViews.push_back(device.createImageView(createInfo));
    }
    return swapchainImageViews;
}

std::vector<vk::raii::Framebuffer> CreateFramebuffers(
    const vk::raii::Device& device,
    const vk::raii::RenderPass& renderPass,
    const std::vector<vk::raii::ImageView>& swapchainImageViews,
    const vk::Extent2D& extent)
{
    std::vector<vk::raii::Framebuffer> framebuffers;
    framebuffers.reserve(swapchainImageViews.size());
    for (const vk::raii::ImageView& imageView : swapchainImageViews)
    {
        vk::FramebufferCreateInfo framebufferCreateInfo{
            .renderPass = *renderPass,
            .attachmentCount = 1,
            .pAttachments = &*imageView,
            .width = extent.width,
            .height = extent.height,
            .layers = 1,
        };
        framebuffers.push_back(device.createFramebuffer(framebufferCreateInfo));
    }
    return framebuffers;
}

void InitVulkanBackend(VulkanBackend& backend, SDL_Window* window)
{
    int windowWidth, windowHeight;
    SDL_GetWindowSizeInPixels(window, &windowWidth, &windowHeight);
    const vk::Extent2D windowSize{static_cast<uint32_t>(windowWidth), static_cast<uint32_t>(windowHeight)};

    backend.instance = CreateInstance(backend.context);
    backend.surface = CreateSurface(backend.instance, window);
    backend.physicalDevice = ChoosePhysicalDevice(backend.instance);
    backend.queueFamily = GetGraphicsQueueFamily(backend.physicalDevice);
    backend.device = CreateDevice(backend.physicalDevice, backend.queueFamily);
    backend.queue = backend.device.getQueue(backend.queueFamily, 0);

    backend.renderPass = CreateRenderPass(backend.device);
    backend.commandPool = backend.device.createCommandPool(
        vk::CommandPoolCreateInfo{
            .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            .queueFamilyIndex = backend.queueFamily,
        });
    auto commandBuffers = backend.device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo{
            .commandPool = backend.commandPool,
            .commandBufferCount = 1,
        });
    ASSUMERT(commandBuffers.size() >= 1);
    backend.commandBuffer = std::move(commandBuffers[0]);
    backend.swapchain = CreateSwapchain(backend.device, backend.surface, windowSize, backend.queueFamily);
    backend.swapchainImages = backend.swapchain.getImages();
    backend.swapchainImageViews = CreateSwapchainImageViews(backend.device, backend.swapchainImages);
    backend.swapchainFramebuffers = CreateFramebuffers(backend.device, backend.renderPass, backend.swapchainImageViews, windowSize);
}

void Renderer::Init(bool launchThread)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("ImGui + SDL + Vulkan", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL3_InitForVulkan(window);

    InitVulkanBackend(vulkanBackend, window);
    ImGui_ImplVulkan_InitInfo vulkanInitInfo{
        .ApiVersion = VK_API_VERSION_1_0,
        .Instance = *vulkanBackend.instance,
        .PhysicalDevice = *vulkanBackend.physicalDevice,
        .Device = *vulkanBackend.device,
        .QueueFamily = vulkanBackend.queueFamily,
        .Queue = *vulkanBackend.queue,
        .DescriptorPool = {},
        .DescriptorPoolSize = 512,
        .MinImageCount = 2,
        .ImageCount = 2,
        .PipelineCache = VK_NULL_HANDLE,
        .PipelineInfoMain = ImGui_ImplVulkan_PipelineInfo{.RenderPass = *vulkanBackend.renderPass, .Subpass = 0},
        .UseDynamicRendering = false,
        .Allocator = nullptr,
        .CheckVkResultFn = &CheckImGuiVulkanResult,
        .MinAllocationSize = 1024 * 1024,
        .CustomShaderVertCreateInfo = vk::ShaderModuleCreateInfo{.sType = static_cast<vk::StructureType>(std::numeric_limits<uint32_t>::max())},
        .CustomShaderFragCreateInfo = vk::ShaderModuleCreateInfo{.sType = static_cast<vk::StructureType>(std::numeric_limits<uint32_t>::max())},
    };
    ImGui_ImplVulkan_Init(&vulkanInitInfo);

    if (launchThread)
        loopThread = std::jthread([this](std::stop_token stopToken) { Loop(stopToken); });
}

void Renderer::Stop()
{
    if (loopThread.joinable())
    {
        Singleton<Logger>::Get().Log("Stopping config renderer loop...");
        loopThread.request_stop();
        loopThread.join();
        Singleton<Logger>::Get().Log("Config renderer loop joined.");
    }

    vulkanBackend.device.waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_Vulkan_DestroySurface(*vulkanBackend.instance, vulkanBackend.surface, nullptr);
    SDL_DestroyWindow(window);
}

void Renderer::LoopTick()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Example Window");
    ImGui::Text("Hello from ImGui with Vulkan!");
    ImGui::End();

    ImGui::Render();

    const auto [result, imageIndex] = vulkanBackend.swapchain.acquireNextImage(1'000'000);

    vulkanBackend.commandBuffer.begin({});
    vulkanBackend.commandBuffer.beginRenderPass(
        vk::RenderPassBeginInfo{.renderPass = vulkanBackend.renderPass, .framebuffer = vulkanBackend.swapchainFramebuffers[imageIndex]},
        vk::SubpassContents::eInline);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *vulkanBackend.commandBuffer);

    vulkanBackend.queue.presentKHR(
        vk::PresentInfoKHR{
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .swapchainCount = 1,
            .pSwapchains = &*vulkanBackend.swapchain,
            .pImageIndices = &imageIndex,
            .pResults = nullptr,
        });

    vulkanBackend.commandBuffer.endRenderPass();
    vulkanBackend.commandBuffer.end();
}

void Renderer::Loop(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
        LoopTick();
}

}  // namespace tektonik::config
