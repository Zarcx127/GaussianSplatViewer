#include "renderer/Renderer.hpp"

#include <chrono>
#include <thread>
#include <algorithm>

#include <glm/glm.hpp>

#include "factory/SplatFrameFactory.hpp"

#include "renderer/resources/descriptors/Descriptors.hpp"

#include "renderer/resources/pipeline/PipelineLayout.hpp"

#include "renderer/resources/splats/SplatSort.hpp"
#include "renderer/resources/splats/SplatFrameDescriptors.hpp"

#include "renderer/FrameCommands.hpp"

namespace
{
    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
    constexpr std::chrono::milliseconds RESIZE_DEBOUNCE_TIME = 
        std::chrono::milliseconds(50);

    constexpr const char* SPLAT_PATH = "splats/scene.ply";
}

Engine::Engine(GLFWwindow* window)
{
    m_logger = Logger::get_logger();
    m_window = window;
}

void Engine::render_loop(AppState& state, const std::function<InputState()>& getInput)
{
    state.renderStatus.store(RenderStatus::Initializing, std::memory_order_release);

    uint32_t initialWidth = 0;
    uint32_t initialHeight = 0;

    while(
        !state.quitRequested.load(std::memory_order_acquire) &&
        ((initialWidth == 0) || (initialHeight == 0))
    ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        initialWidth = static_cast<uint32_t>(
            state.framebufferWidth.load(std::memory_order_acquire)
        );

        initialHeight = static_cast<uint32_t>(
            state.framebufferHeight.load(std::memory_order_acquire)
        );
    }

    if(state.quitRequested.load(std::memory_order_acquire))
    {
        state.renderStatus.store(RenderStatus::Stopped, std::memory_order_release);
        return;
    }

    if(!init(initialWidth, initialHeight))
    {
        state.renderStatus.store(RenderStatus::InitFailed, std::memory_order_release);
        state.quitRequested.store(true, std::memory_order_release);

        glfwPostEmptyEvent();
        return;
    }

    state.renderStatus.store(RenderStatus::Running, std::memory_order_release);
    
    uint64_t previousResizeGeneration = state.resizeGeneration.load(std::memory_order_acquire);

    using Clock = std::chrono::steady_clock;

    bool resizePending = false;
    Clock::time_point lastResizeEventTime = Clock::now();

    while(!state.quitRequested.load(std::memory_order_acquire))
    {
        int width = state.framebufferWidth.load(std::memory_order_relaxed);
        int height = state.framebufferHeight.load(std::memory_order_relaxed);

        if((width <= 0) || (height <= 0))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        uint64_t currentResizeGeneration = 
            state.resizeGeneration.load(std::memory_order_acquire);

        if(currentResizeGeneration != previousResizeGeneration)
        {
            previousResizeGeneration = currentResizeGeneration;
            lastResizeEventTime = Clock::now();
            resizePending = true;
        }

        if(resizePending)
        {
            Clock::time_point timeNow  = Clock::now();

            if((timeNow - lastResizeEventTime) >= RESIZE_DEBOUNCE_TIME)
            {
                m_rebuildSwapchain = true;
                resizePending = false;
            }
        }

        InputState input = getInput();
        DrawResult result = draw(
            static_cast<uint32_t>(width), 
            static_cast<uint32_t>(height),
            input
        );

        if(result == DrawResult::Success)
        {
            previousResizeGeneration = currentResizeGeneration;
        }
        else if(result == DrawResult::FatalError)
        {
            state.renderStatus.store(RenderStatus::FatalError, std::memory_order_release);
            state.quitRequested.store(true, std::memory_order_release);

            glfwPostEmptyEvent();
            break;
        }

        update_timing(state);
    }

    state.renderStatus.store(RenderStatus::Stopping, std::memory_order_release);
    shutdown();
    state.renderStatus.store(RenderStatus::Stopped, std::memory_order_release);
}

bool Engine::init(uint32_t width, uint32_t height)
{
    m_initialized = true;

    bool vulkanContextBuildSuccessful = 
        m_vulkanContext.build(m_window, "Graphics Engine");

    if(!vulkanContextBuildSuccessful)
    {
        shutdown();
        return false;
    }

    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder(
        m_vulkanContext.logical_device_ref()
    );
    
    descriptorSetLayoutBuilder.add_entry(
        vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageImage
    );

    m_descriptorSetLayout = descriptorSetLayoutBuilder.build(m_interfaceDeletionQueue);
    if(!m_descriptorSetLayout)
    {
        shutdown();
        return false;
    }
    
    m_renderInterface.add_descriptor_set_layout(m_descriptorSetLayout);

    DescriptorSetLayoutBuilder sphericalHarmonicLayoutBuilder(
        m_vulkanContext.logical_device_ref()
    );

    sphericalHarmonicLayoutBuilder.add_entry(
        (
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eVertex
        ),
        vk::DescriptorType::eStorageBuffer
    );

    m_sphericalHarmonicDescriptorSetLayout = 
        sphericalHarmonicLayoutBuilder.build(m_interfaceDeletionQueue);

    if(!m_sphericalHarmonicDescriptorSetLayout)
    {
        shutdown();
        return false;
    }

    m_renderInterface.add_descriptor_set_layout(
        m_sphericalHarmonicDescriptorSetLayout
    );

    m_splatFrameDescriptorSetLayout = build_splat_frame_descriptor_set_layout(
        m_vulkanContext.logical_device_ref(),
        m_interfaceDeletionQueue
    );
        
    if(!m_splatFrameDescriptorSetLayout)
    {
        shutdown();
        return false;
    }

    m_renderInterface.add_descriptor_set_layout(m_splatFrameDescriptorSetLayout);

    m_renderInterface.add_push_constant_range(
        vk::ShaderStageFlagBits::eVertex,
        0U, 
        sizeof(FramePushConstant)
    );

    m_renderInterface.add_push_constant_range(
        vk::ShaderStageFlagBits::eCompute,
        0U, 
        (
            sizeof(FramePushConstant) +
            sizeof(SplatSortPushConstant)
        )
    );

    PipelineLayoutBuilder pipelineLayoutBuilder(m_vulkanContext.logical_device_ref());

    m_pipelineLayout = pipelineLayoutBuilder.build(m_renderInterface, m_interfaceDeletionQueue);
    if(!m_pipelineLayout)
    {
        shutdown();
        return false;
    }

    if(!init_render_features())
    {
        shutdown();
        return false;    
    }

    if(!init_render_resources(width, height))
    {
        shutdown();
        return false;
    }

    m_currentTime = glfwGetTime();
    m_lastTime = m_currentTime;
    m_lastFrameTime = m_currentTime;
    m_numFrames = 0;

    m_logger->print("Graphics engine started");
    
    return true;
}

Engine::DrawResult Engine::draw(uint32_t width, uint32_t height, const InputState& input)
{
    if(m_rebuildSwapchain)
    {
        destroy_render_resources();
        if(!init_render_resources(width, height))
            return DrawResult::Skipped;

        m_currFrame = 0;
        m_rebuildSwapchain = false;
    }

    if(m_currFrame >= m_renderResources.frame_count())
    {
        m_logger->print("Invalid current frame index");
        return DrawResult::FatalError;
    }

    vk::Device device = m_vulkanContext.logical_device();
    vk::Queue graphicsQueue = m_vulkanContext.graphics_queue();

    Frame& frame = m_renderResources.frame(m_currFrame);

    if(
        device.waitForFences(
            frame.renderFinishedFence, 
            vk::False, UINT64_MAX
        ) != vk::Result::eSuccess
    ) {
        m_logger->print("Failed to wait for current frame fence");
        return DrawResult::FatalError;
    }

    GpuSplatCounters splatCounters = {};
    bool readSplatCountersSuccess = read_splat_frame_counters(
        m_vulkanContext.allocator(),
        frame.splatResources,
        splatCounters
    );

    if(!readSplatCountersSuccess)
    {
        m_logger->print("Failed to read splat counters");
        return DrawResult::FatalError;
    }

    if(splatCounters.overflowCount > 0)
    {
        uint32_t grownCapacity = calculate_grown_splat_entry_capacity(
            m_vulkanContext.physical_device(),
            frame.splatResources.entryCapacity,
            splatCounters.requestedEntryCount
        );

        if(grownCapacity <= frame.splatResources.entryCapacity) 
        {
            m_logger->print("Splat entry capacity cannot grow further");
            return DrawResult::FatalError;
        }

        m_logger->print("Growing splat entry capacity");

        m_splatEntryCapacity = grownCapacity;

        destroy_render_resources();
        if(!init_render_resources(width, height))
            return DrawResult::FatalError;

        m_currFrame = 0;

        return DrawResult::Skipped;
    }

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(
        static_cast<VkDevice>(device),
        static_cast<VkSwapchainKHR>(m_renderResources.swapchain_handle()),
        UINT64_MAX,
        static_cast<VkSemaphore>(frame.imageAcquiredSemaphore),
        VK_NULL_HANDLE,
        &imageIndex
    );

    if(acquireResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_rebuildSwapchain = true;
        return DrawResult::NeedsSwapchainRebuild;
    }

    if(acquireResult == VK_SUBOPTIMAL_KHR)
    {
        m_rebuildSwapchain = true;
    }
    else if(acquireResult != VK_SUCCESS)
    {
        m_logger->print("Failed to acquire swapchain image");
        return DrawResult::FatalError;
    }

    double now = glfwGetTime();
    
    float deltaTime = static_cast<float>(now - m_lastFrameTime);
    m_lastFrameTime = now;

    deltaTime = std::min(deltaTime, 0.1f);

    m_camera.update(deltaTime, input);

    vk::Extent2D renderExtent = m_renderResources.extent();
    float aspect = (
        static_cast<float>(renderExtent.width) /
        static_cast<float>(renderExtent.height)
    );

    RenderFeatureFrameInfo featureInfo = m_renderFeatures.frame_info();

    FramePushConstant pushConstants {};

    pushConstants.view = m_camera.view_matrix();
    pushConstants.cameraPosition = glm::vec4(m_camera.position(), 1.0f);
    
    glm::mat4 projection = m_camera.projection_matrix(aspect);
    pushConstants.projectionInfo = glm::vec4(
        projection[0][0],
        projection[1][1],
        projection[2][2],
        projection[3][2]
    );
    
    pushConstants.renderInfo = glm::uvec4(
        renderExtent.width,
        renderExtent.height,
        featureInfo.sphericalHarmonicBuffer.degree,
        featureInfo.sphericalHarmonicBuffer.coefficientCount
    );

    if(
        (imageIndex >= m_renderResources.color_image_count()) ||
        (imageIndex >= m_renderResources.color_image_view_count()) ||
        (imageIndex >= m_renderResources.image_in_flight_count()) ||
        (imageIndex >= m_renderResources.swapchain_storage_descriptor_set_count())
    ) {
        m_logger->print("Invalid acquired swapchain image index");
        return DrawResult::FatalError;
    }

    vk::Fence& imageInFlight = m_renderResources.image_in_flight(imageIndex);
    if(imageInFlight)
    {
        if(
            device.waitForFences(
                1, &(imageInFlight), 
                vk::True, UINT64_MAX
            ) != vk::Result::eSuccess
        ) {
            m_logger->print("Failed to wait for swapchain image fence");
            return DrawResult::FatalError;
        }
    }

    imageInFlight = frame.renderFinishedFence;
    if(device.resetFences(frame.renderFinishedFence) != vk::Result::eSuccess)
    {
        m_logger->print("Failed to reset current frame fence");
        return DrawResult::FatalError;
    }
    
    RenderTarget renderTarget = {
        m_renderResources.color_image(imageIndex),
        m_renderResources.color_image_view(imageIndex),
        renderExtent,
        m_renderResources.swapchain_storage_descriptor_set(imageIndex),
        m_renderResources.depth_image()
    };

    FrameCommands FrameCommands(
        frame.commandBuffer,
        renderTarget,
        frame.splatResources,
        frame.splatFrameDescriptorSet,
        m_renderResources.splat_gaussian_pipeline(),
        m_pipelineLayout,
        featureInfo
    );

    if(!FrameCommands.record(pushConstants))
        return DrawResult::FatalError;
    
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eComputeShader;
    vk::SubmitInfo submitInfo = {};

    submitInfo.commandBufferCount = 1;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.signalSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &(frame.commandBuffer);
    submitInfo.pWaitSemaphores = &(frame.imageAcquiredSemaphore);
    submitInfo.pSignalSemaphores = &(frame.renderFinishedSemaphores[imageIndex]);
    submitInfo.pWaitDstStageMask = &waitStage;

    if(graphicsQueue.submit(submitInfo, frame.renderFinishedFence) != vk::Result::eSuccess)
    {
        m_logger->print("Failed to submit frame command buffer");
        return DrawResult::FatalError;
    }

    VkSwapchainKHR rawSwapchain = static_cast<VkSwapchainKHR>(
        m_renderResources.swapchain_handle()
    );

    VkSemaphore rawWaitSemaphore = 
        static_cast<VkSemaphore>(frame.renderFinishedSemaphores[imageIndex]);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &rawWaitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &rawSwapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    VkResult presentResult = vkQueuePresentKHR(
        static_cast<VkQueue>(graphicsQueue),
        &presentInfo
    );

    if(
        (presentResult == VK_ERROR_OUT_OF_DATE_KHR) ||
        (presentResult == VK_SUBOPTIMAL_KHR)
    ) {
        m_rebuildSwapchain = true;
        return DrawResult::NeedsSwapchainRebuild;
    }

    if(presentResult != VK_SUCCESS)
    {
        m_logger->print("Failed to present swapchain image");
        return DrawResult::FatalError;
    }

    m_currFrame = (m_currFrame + 1) % m_renderResources.frame_count();

    return DrawResult::Success;
}

void Engine::update_timing(AppState& state)
{
    m_numFrames++;

    m_currentTime = glfwGetTime();
    double delta = m_currentTime - m_lastTime;

    if(delta >= 1)
    {
        uint32_t frameRate = std::max(1U, uint32_t(m_numFrames / delta));
        state.fps.store(frameRate, std::memory_order_relaxed);

        m_lastTime = m_currentTime;
        
        m_numFrames = 0;
    }
}

bool Engine::init_render_features()
{
    RenderFeaturesContext context = make_render_features_context();
    return m_renderFeatures.build(context, SPLAT_PATH);
}

void Engine::destroy_render_features()
{
    RenderFeaturesContext context = make_render_features_context();
    m_renderFeatures.destroy(context);
}

bool Engine::init_render_resources(uint32_t width, uint32_t height)
{
    RenderFeatureFrameInfo featureInfo = m_renderFeatures.frame_info();

    uint32_t splatCapacity = featureInfo.splatBuffer.splatCount;

    if(m_splatEntryCapacity == 0)
    {
        m_splatEntryCapacity = calculate_splat_entry_capacity(
            m_vulkanContext.physical_device(),
            splatCapacity, 
            MAX_FRAMES_IN_FLIGHT
        );
    }

    if(m_splatEntryCapacity == 0)
        return false;

    RenderResourcesContext context = make_render_resources_context();

    return m_renderResources.build(
        context, width, height, MAX_FRAMES_IN_FLIGHT,
        featureInfo.splatBuffer, m_splatEntryCapacity
    );
}

void Engine::destroy_render_resources()
{
    RenderResourcesContext context = make_render_resources_context();
    m_renderResources.destroy(context);
}

RenderFeaturesContext Engine::make_render_features_context()
{
    return {
        m_vulkanContext.logical_device(),
        m_vulkanContext.allocator(),
        m_vulkanContext.command_pool(),
        m_vulkanContext.graphics_queue(),
        m_pipelineLayout,
        m_sphericalHarmonicDescriptorSetLayout
    };
}

RenderResourcesContext Engine::make_render_resources_context()
{
    return {
        m_vulkanContext.physical_device(),
        m_vulkanContext.logical_device(),
        m_vulkanContext.surface(),
        m_vulkanContext.allocator(),
        m_vulkanContext.command_pool(),
        m_vulkanContext.graphics_queue(),
        m_splatFrameDescriptorSetLayout,
        m_pipelineLayout,
        m_renderInterface
    };
}

void Engine::shutdown()
{
    if(!m_initialized) return;

    destroy_render_resources();
    destroy_render_features();

    vk::Device device = m_vulkanContext.logical_device();
    while(!m_interfaceDeletionQueue.empty())
    {
        if(device)
            (m_interfaceDeletionQueue.back())(device);
    
        m_interfaceDeletionQueue.pop_back();
    }

    m_descriptorSetLayout = vk::DescriptorSetLayout();
    m_sphericalHarmonicDescriptorSetLayout = vk::DescriptorSetLayout();
    m_splatFrameDescriptorSetLayout = vk::DescriptorSetLayout();

    m_pipelineLayout = vk::PipelineLayout();
    m_renderInterface = ShaderInterface();

    m_vulkanContext.destroy();

    m_initialized = false;
}

Engine::~Engine()
{
    shutdown();
    m_logger->print("Deleted graphics engine");
}
