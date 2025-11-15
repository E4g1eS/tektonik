module;
#include "imgui-wrapper.hpp"
#include "sdl-wrapper.hpp"
#include "common-defines.hpp"
module config_renderer;

import singleton;
import util;
import logger;
import vulkan_util;
import util;
import vulkan_hpp;
import std;

namespace tektonik::config
{

constexpr uint32_t kMaxFramesInFLight = 2;

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

std::vector<vk::raii::Semaphore> CreateSemaphores(const vk::raii::Device& device, const size_t count)
{
    std::vector<vk::raii::Semaphore> semaphores;
    semaphores.reserve(count);
    for (size_t i = 0; i < count; ++i)
        semaphores.push_back(device.createSemaphore(vk::SemaphoreCreateInfo{}));
    return semaphores;
}

std::vector<vk::raii::Fence> CreateFences(const vk::raii::Device& device, const size_t count, const bool signaled = false)
{
    std::vector<vk::raii::Fence> fences;
    fences.reserve(count);
    for (size_t i = 0; i < count; ++i)
        fences.push_back(device.createFence(vk::FenceCreateInfo{.flags = signaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags{}}));
    return fences;
}

vk::Extent2D GetSurfaceExtent(const vk::raii::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface)
{
    return physicalDevice.getSurfaceCapabilitiesKHR(surface).currentExtent;
}

VulkanBackend::SwapchainWrapper CreateSwapchainWrapper(
    const vk::raii::Device& device,
    const vk::SurfaceKHR& surface,
    const vk::Extent2D& windowSize,
    const uint32_t queueFamily,
    const vk::raii::RenderPass& renderPass,
    const vk::raii::CommandPool& commandPool)
{
    VulkanBackend::SwapchainWrapper swapchainWrapper;
    swapchainWrapper.extent = windowSize;
    swapchainWrapper.swapchain = CreateSwapchain(device, surface, swapchainWrapper.extent, queueFamily);
    swapchainWrapper.images = swapchainWrapper.swapchain.getImages();
    swapchainWrapper.imageViews = CreateSwapchainImageViews(device, swapchainWrapper.images);
    swapchainWrapper.framebuffers = CreateFramebuffers(device, renderPass, swapchainWrapper.imageViews, swapchainWrapper.extent);
    swapchainWrapper.submitFinishedSemaphores = CreateSemaphores(device, swapchainWrapper.images.size());

    swapchainWrapper.acquiredImageSemaphores = CreateSemaphores(device, kMaxFramesInFLight);
    swapchainWrapper.commandBuffers =
        device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{.commandPool = commandPool, .commandBufferCount = kMaxFramesInFLight});
    swapchainWrapper.submitFinishedFences = CreateFences(device, kMaxFramesInFLight, true);

    return swapchainWrapper;
}

void InitVulkanBackend(VulkanBackend& backend, SDL_Window* window)
{
    backend.instance = CreateInstance(backend.context);
    backend.surface = SurfaceWrapper(backend.instance, window);
    backend.physicalDevice = ChoosePhysicalDevice(backend.instance);
    backend.queueFamily = GetGraphicsQueueFamily(backend.physicalDevice);
    backend.device = CreateDevice(backend.physicalDevice, backend.queueFamily);
    backend.queue = backend.device.getQueue(backend.queueFamily, 0);

    backend.renderPass = CreateRenderPass(backend.device);
    backend.commandPool = backend.device.createCommandPool(
        vk::CommandPoolCreateInfo{.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer, .queueFamilyIndex = backend.queueFamily});

    backend.swapchainWrapper = CreateSwapchainWrapper(
        backend.device,
        *backend.surface,
        GetSurfaceExtent(backend.physicalDevice, *backend.surface),
        backend.queueFamily,
        backend.renderPass,
        backend.commandPool);
}

SurfaceWrapper::SurfaceWrapper(const vk::raii::Instance& instance, SDL_Window* window) : instance(instance)
{
    VkSurfaceKHR cSurface;
    if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &cSurface))
        throw std::runtime_error("Could not create a Vulkan surface.");

    surface = cSurface;
}

SurfaceWrapper::~SurfaceWrapper()
{
    if (instance && surface)
        SDL_Vulkan_DestroySurface(instance, surface, nullptr);
}

Renderer::Renderer(Manager& manager) : manager(&manager)
{
    window = SDL_CreateWindow("ImGui + SDL + Vulkan", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    IMGUI_CHECKVERSION();
    imGuiContext = ImGui::CreateContext();

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
}

Renderer::~Renderer()
{
    if (!manager)
        return;

    vulkanBackend.device.waitIdle();
    ImGui_ImplVulkan_Shutdown();

    util::MoveDelete(vulkanBackend);

    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(window);
}

void Renderer::Tick()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    AddImGuiThings();

    ImGui::Render();

    VulkanTick();
}

void Renderer::AddImGuiThings()
{
    auto& variables = manager->GetVariables();

    ImGui::Begin("Variables");

    static String debugString("DebugString", "Hello config");

    for (const auto& [name, value] : variables)
    {
        std::visit([](auto&& arg)
            {
            using T = std::remove_cvref_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, String*>)
                ImGui::Text((**arg).c_str());
            }, value);
    }

    ImGui::End();
}

void Renderer::VulkanTick()
{
    constexpr uint64_t kTimeoutNs = 1'000'000'000ULL;

    VulkanBackend::SwapchainWrapper& swapchainWrapper = vulkanBackend.swapchainWrapper;
    size_t& currentFrameIndex = swapchainWrapper.currentFrameIndex;

    if (!*swapchainWrapper.swapchain)
    {
        RecreateSwapchain();
        if (!*swapchainWrapper.swapchain)
            return;
    }

    static_cast<void>(vulkanBackend.device.waitForFences(*swapchainWrapper.submitFinishedFences[currentFrameIndex], true, kTimeoutNs));
    vulkanBackend.device.resetFences(*swapchainWrapper.submitFinishedFences[currentFrameIndex]);

    try
    {
        const auto [result, imageIndex] =
            swapchainWrapper.swapchain.acquireNextImage(kTimeoutNs, swapchainWrapper.acquiredImageSemaphores[currentFrameIndex]);

        if (result == vk::Result::eSuboptimalKHR)
        {
            Singleton<Logger>::Get().Log("Swapchain is suboptimal.");
            RecreateSwapchain();
            return;
        }

        vk::raii::CommandBuffer& commandBuffer = swapchainWrapper.commandBuffers[currentFrameIndex];

        commandBuffer.reset();
        commandBuffer.begin({});

        vk::ClearValue clearValue = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));

        commandBuffer.beginRenderPass(
            vk::RenderPassBeginInfo{
                .renderPass = vulkanBackend.renderPass,
                .framebuffer = swapchainWrapper.framebuffers[currentFrameIndex],
                .renderArea =
                    vk::Rect2D{
                               .offset = vk::Offset2D{0, 0},
                               .extent = swapchainWrapper.extent,
                               },
                .clearValueCount = 1,
                .pClearValues = &clearValue,
        },
            vk::SubpassContents::eInline);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);

        commandBuffer.endRenderPass();
        commandBuffer.end();

        static constexpr vk::PipelineStageFlags kWaitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

        vulkanBackend.queue.submit(
            vk::SubmitInfo{
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &*swapchainWrapper.acquiredImageSemaphores[currentFrameIndex],
                .pWaitDstStageMask = &kWaitStage,
                .commandBufferCount = 1,
                .pCommandBuffers = &*commandBuffer,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &*swapchainWrapper.submitFinishedSemaphores[imageIndex],
            },
            swapchainWrapper.submitFinishedFences[currentFrameIndex]);

        static_cast<void>(vulkanBackend.queue.presentKHR(
            vk::PresentInfoKHR{
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &*swapchainWrapper.submitFinishedSemaphores[imageIndex],
                .swapchainCount = 1,
                .pSwapchains = &*swapchainWrapper.swapchain,
                .pImageIndices = &imageIndex,
                .pResults = nullptr,
            }));

        swapchainWrapper.currentFrameIndex = (swapchainWrapper.currentFrameIndex + 1) % kMaxFramesInFLight;
    }
    catch (const vk::OutOfDateKHRError&)
    {
        Singleton<Logger>::Get().Log("Swapchain is out of date.");
        RecreateSwapchain();
    }
}

void Renderer::RecreateSwapchain()
{
    vulkanBackend.device.waitIdle();

    util::MoveDelete(vulkanBackend.swapchainWrapper);

    vk::Extent2D windowSize = GetSurfaceExtent(vulkanBackend.physicalDevice, *vulkanBackend.surface);
    if (windowSize.width == 0 && windowSize.height == 0)
        return;

    Singleton<Logger>::Get().Log("Recreating swapchain...");

    vulkanBackend.swapchainWrapper = CreateSwapchainWrapper(
        vulkanBackend.device,
        *vulkanBackend.surface,
        GetSurfaceExtent(vulkanBackend.physicalDevice, *vulkanBackend.surface),
        vulkanBackend.queueFamily,
        vulkanBackend.renderPass,
        vulkanBackend.commandPool);
}

}  // namespace tektonik::config
