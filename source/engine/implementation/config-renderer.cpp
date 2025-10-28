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

    vk::AttachmentDescription depthAttachment{
        .format = vk::Format::eD32Sfloat,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,  // Dont care about stencil buffer
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
    };

    vk::AttachmentReference depthAttachmentReference{
        .attachment = 1,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
    };

    vk::SubpassDescription subpass{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pDepthStencilAttachment = &depthAttachmentReference,
    };

    vk::SubpassDependency subpassDependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
    };

    auto attachments = std::array{colorAttachment, depthAttachment};

    vk::RenderPassCreateInfo createInfo{
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
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

vk::raii::SwapchainKHR CreateSwapchain(const vk::raii::Device& device, const vk::SurfaceKHR& surface)
{
    return device.createSwapchainKHR(vk::SwapchainCreateInfoKHR{});
}

void InitVulkanBackend(VulkanBackend& backend, SDL_Window* window)
{
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
    backend.swapchain = CreateSwapchain(backend.device, backend.surface);
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

    vulkanBackend.commandBuffer.begin({});
    vulkanBackend.commandBuffer.beginRenderPass(vk::RenderPassBeginInfo{.renderPass = vulkanBackend.renderPass}, vk::SubpassContents::eInline);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *vulkanBackend.commandBuffer);

    vulkanBackend.commandBuffer.endRenderPass();
    vulkanBackend.commandBuffer.end();
}

void Renderer::Loop(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
        LoopTick();
}

}  // namespace tektonik::config
