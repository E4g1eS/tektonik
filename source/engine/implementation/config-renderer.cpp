module;
#include "imgui-wrapper.hpp"
#include "sdl-wrapper.hpp"
#include "std.hpp"
#include "vulkan-wrapper.hpp"
module config_renderer;

import singleton;
import logger;

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

void InitVulkanBackend(VulkanBackend& vulkanBackend)
{
    vulkanBackend.instance = vulkanBackend.context.createInstance(vk::InstanceCreateInfo{});

    std::vector<vk::raii::PhysicalDevice> physicalDevices = vulkanBackend.instance.enumeratePhysicalDevices();
    ASSUMERT(physicalDevices.size() >= 1);
    vulkanBackend.physicalDevice = physicalDevices[0];  // TODO

    vulkanBackend.queueFamily = GetGraphicsQueueFamily(vulkanBackend.physicalDevice);
    vk::DeviceQueueCreateInfo queueCreateInfo{.queueFamilyIndex = vulkanBackend.queueFamily, .queueCount = 1};
    vulkanBackend.device = vulkanBackend.physicalDevice.createDevice(
        vk::DeviceCreateInfo{
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueCreateInfo,
        });

    vulkanBackend.queue = vulkanBackend.device.getQueue(vulkanBackend.queueFamily, 0);
    vulkanBackend.renderPass = CreateRenderPass(vulkanBackend.device);
    vulkanBackend.commandPool = vulkanBackend.device.createCommandPool(vk::CommandPoolCreateInfo{.queueFamilyIndex = vulkanBackend.queueFamily});
    auto commandBuffers = vulkanBackend.device.allocateCommandBuffers(
        vk::CommandBufferAllocateInfo{
            .commandPool = vulkanBackend.commandPool,
            .commandBufferCount = 1,
        });
    ASSUMERT(commandBuffers.size() >= 1);
    vulkanBackend.commandBuffer = std::move(commandBuffers[0]);
}

void Renderer::Init(bool launchThread)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("ImGui + SDL + Vulkan", 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplSDL3_InitForVulkan(window);

    InitVulkanBackend(vulkanBackend);
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
    Singleton<Logger>::Get().Log("Stopping config renderer loop...");

    loopThread.request_stop();
    ASSUMERT(loopThread.joinable());
    loopThread.join();

    Singleton<Logger>::Get().Log("Config renderer loop joined.");

    vulkanBackend.device.waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
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
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *vulkanBackend.commandBuffer);
}

void Renderer::Loop(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
        LoopTick();
}

}  // namespace tektonik::config
