#include "renderer/Renderer.hpp"

#include <vector>
#include <chrono>
#include <thread>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "factory/MeshFactory.hpp"

#include "renderer/core/Image.hpp"
#include "renderer/core/Device.hpp"
#include "renderer/core/Command.hpp"
#include "renderer/core/Instance.hpp"
#include "renderer/core/Swapchain.hpp"
#include "renderer/core/Synchronization.hpp"

#include "renderer/resources/Allocator.hpp"
#include "renderer/resources/descriptors/Descriptors.hpp"
#include "renderer/resources/pipeline/PipelineLayout.hpp"
#include "renderer/resources/shaders/Shader.hpp"

Engine::Engine(GLFWwindow* window)
{
    m_logger = Logger::get_logger();
    m_window = window;
}

void Engine::render_loop(AppState& state)
{
    state.renderStatus.store(RenderStatus::Initializing, std::memory_order_release);

    uint32_t initialWidth = 
        static_cast<uint32_t>(state.framebufferWidth.load(std::memory_order_acquire));

    uint32_t initialHeight = 
        static_cast<uint32_t>(state.framebufferHeight.load(std::memory_order_acquire));

    if(!init(initialWidth, initialHeight))
    {
        state.renderStatus.store(RenderStatus::InitFailed, std::memory_order_release);
        state.quitRequested.store(true, std::memory_order_release);

        glfwPostEmptyEvent();
        return;
    }

    state.renderStatus.store(RenderStatus::Running, std::memory_order_release);
    
    uint64_t previousResizeGeneration = state.resizeGeneration.load(std::memory_order_acquire);

    while(!state.quitRequested.load(std::memory_order_acquire))
    {
        int width = state.framebufferWidth.load(std::memory_order_relaxed);
        int height = state.framebufferHeight.load(std::memory_order_relaxed);

        if(width <= 0 || height <= 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        uint64_t currentResizeGeneration = 
            state.resizeGeneration.load(std::memory_order_acquire);

        if(currentResizeGeneration != previousResizeGeneration)
        {
            m_rebuildSwapchain = true;
        }

        DrawResult result = draw(
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
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

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    
    m_instance = make_instance("VK Engine", m_instanceDeletionQueue);
    if(!m_instance)
    {
        shutdown();
        return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

#ifdef DEBUG
    
    m_debugMessenger = m_logger->make_debug_messenger(m_instance, m_instanceDeletionQueue);
    if(!m_debugMessenger && m_logger->is_enabled())
    {
        shutdown();
        return false;
    }

#endif

    VkSurfaceKHR rawSurface;
    VkResult rawSurfaceResult = glfwCreateWindowSurface(m_instance, m_window, nullptr, &rawSurface);
    if(rawSurfaceResult != VK_SUCCESS)
    {
        m_logger->print("Failed to create window surface");
        
        shutdown();
        return false;
    }

    m_surface = vk::SurfaceKHR(rawSurface);
    m_instanceDeletionQueue.push_back(
        [this] (vk::Instance instance)->void {
            instance.destroySurfaceKHR(m_surface);
            m_logger->print("Deleted Vulkan surface");
        }
    );

    m_logger->set_mode(false);
    m_physicalDevice = choose_physical_device(m_instance);
    if(!m_physicalDevice)
    {
        shutdown();
        return false;
    }
    
    m_logicalDevice = create_logical_device(m_physicalDevice, m_surface, m_deviceDeletionQueue);
    if(!m_logicalDevice)
    {
        shutdown();
        return false;
    }

    m_graphicsQueueFamilyIndex = find_queue_family_index(
        m_physicalDevice, m_surface, vk::QueueFlagBits::eGraphics 
    );

    if(m_graphicsQueueFamilyIndex == UINT32_MAX)
    {
        m_logger->print("No graphics queue found");

        shutdown();
        return false;
    }

    m_graphicsQueue = m_logicalDevice.getQueue(m_graphicsQueueFamilyIndex, 0);
    m_commandPool = make_command_pool(
        m_logicalDevice, m_graphicsQueueFamilyIndex, m_deviceDeletionQueue
    );

    if(!m_commandPool)
    {
        shutdown();
        return false;
    }

    m_allocator = make_vma_allocator(m_instance, m_physicalDevice, m_logicalDevice);
    if(!m_allocator)
    {
        shutdown();
        return false;
    }

    m_mesh = build_cube(
        m_allocator, m_logicalDevice, m_commandPool, m_graphicsQueue, m_vmaDeletionQueue
    );

    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder(m_logicalDevice);
    descriptorSetLayoutBuilder.add_entry(
        vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageImage
    );

    vk::DescriptorSetLayout descriptorSetLayout = descriptorSetLayoutBuilder.build(m_deviceDeletionQueue);
    if(!descriptorSetLayout)
    {
        shutdown();
        return false;
    }
    
    m_graphicsInterface.add_descriptor_set_layout(descriptorSetLayout);
    m_graphicsInterface.add_push_constant_range(
        vk::ShaderStageFlagBits::eVertex, 0U, sizeof(glm::mat4)
    );

    PipelineLayoutBuilder pipelineLayoutBuilder(m_logicalDevice);

    m_pipelineLayout = pipelineLayoutBuilder.build(m_graphicsInterface, m_deviceDeletionQueue);
    if(!m_pipelineLayout)
    {
        shutdown();
        return false;
    }

    m_computeShader = make_compute_shader(
        m_logicalDevice, "shaders/bin/clear_screen.comp.spv", m_graphicsInterface, m_deviceDeletionQueue
    );

    if(!m_computeShader)
    {
        shutdown();
        return false;
    }

    m_shaders = make_shader_object(
        m_logicalDevice, 
        "shaders/bin/shader.vert.spv", 
        "shaders/bin/shader.frag.spv", 
        m_graphicsInterface, 
        m_deviceDeletionQueue
    );

    if(m_shaders.empty())
    {
        shutdown();
        return false;
    }

    m_frames.reserve(3);
    if(!init_render_resources(width, height))
    {
        shutdown();
        return false;
    }

    m_currentTime = glfwGetTime();
    m_lastTime = m_currentTime;
    m_numFrames = 0;

    m_logger->print("Graphics engine started");
    return true;
}

Engine::DrawResult Engine::draw(uint32_t width, uint32_t height)
{
    if(m_rebuildSwapchain)
    {
        (void) m_logicalDevice.waitIdle();

        destroy_render_resources();
        if(!init_render_resources(width, height))
            return DrawResult::Skipped;

        m_currFrame = 0;
        m_rebuildSwapchain = false;
    }

    Frame& frame = m_frames[m_currFrame];

    (void) m_logicalDevice.waitForFences(frame.renderFinishedFence, vk::False, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(
        static_cast<VkDevice>(m_logicalDevice),
        static_cast<VkSwapchainKHR>(m_swapchain.chain),
        UINT64_MAX,
        static_cast<VkSemaphore>(frame.imageacquiredSemaphore),
        VK_NULL_HANDLE,
        &imageIndex
    );

    if(
        (acquireResult == VK_ERROR_OUT_OF_DATE_KHR) ||
        (acquireResult == VK_SUBOPTIMAL_KHR)
    ) {
        m_rebuildSwapchain = true;
        return DrawResult::NeedsSwapchainRebuild;
    }

    if(acquireResult != VK_SUCCESS)
    {
        m_logger->print("Failed to acquire swapchain image");
        return DrawResult::FatalError;
    }

    if(m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        (void) m_logicalDevice.waitForFences(1, &(m_imagesInFlight[imageIndex]), vk::True, UINT64_MAX);

    m_imagesInFlight[imageIndex] = frame.renderFinishedFence;
    (void) m_logicalDevice.resetFences(frame.renderFinishedFence);
    
    float aspect = 
        (static_cast<float>(m_swapchain.extent.width) / static_cast<float>(m_swapchain.extent.height));

    float t = glfwGetTime();

    glm::mat4 model = glm::rotate(
        glm::mat4(1.0f),
        t,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        aspect,
        0.1f,
        10.0f
    );

    proj[1][1] *= -1.0f;

    glm::mat4 mvp = proj * view * model;

    frame.record_command_buffer(imageIndex, mvp);
    
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo = {};

    submitInfo.commandBufferCount = 1;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.signalSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &(frame.commandBuffer);
    submitInfo.pWaitSemaphores = &(frame.imageacquiredSemaphore);
    submitInfo.pSignalSemaphores = &(frame.renderFinishedSemaphores[imageIndex]);
    submitInfo.pWaitDstStageMask = &waitStage;

    (void) m_graphicsQueue.submit(submitInfo, frame.renderFinishedFence);

    VkSwapchainKHR rawSwapchain = static_cast<VkSwapchainKHR>(m_swapchain.chain);
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
        static_cast<VkQueue>(m_graphicsQueue),
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

    m_currFrame = (m_currFrame + 1) % m_frames.size();

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
        m_frameTime = 1000.0 / frameRate;
    }
}

bool Engine::init_render_resources(uint32_t width, uint32_t height)
{
    if((width == 0) || (height == 0)) 
        return false;

    DescriptorPoolBuilder descriptorPoolBuilder(m_logicalDevice);
    descriptorPoolBuilder.add_entry(vk::DescriptorType::eStorageImage);

    m_descriptorPool = descriptorPoolBuilder.build(3, m_renderDeletionQueue);
    if(!m_descriptorPool)
    {
        m_logger->print("Rendering crashed");
        destroy_render_resources();
        
        return false;
    }

    m_swapchain.build(m_logicalDevice, m_physicalDevice, m_surface, width, height, m_renderDeletionQueue);
    if(m_swapchain.imageViews.empty())
    {
        m_logger->print("Rendering crashed");
        destroy_render_resources();

        return false;
    }

    m_depthImage = create_depth_image(
        m_allocator,m_logicalDevice, m_physicalDevice, m_swapchain.extent, 
        m_renderDeletionQueue, m_renderVmaDeletionQueue
    );

    if(!m_depthImage.image || !m_depthImage.imageView)
    {
        m_logger->print("Rendering crashed");
        destroy_render_resources();

        return false;
    }

    immediate_submit(
        m_logicalDevice, m_commandPool, m_graphicsQueue,
        [this] (vk::CommandBuffer commandBuffer)->void {
            transition_image_layout(
                commandBuffer,
                m_depthImage.image,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthAttachmentOptimal,
                vk::AccessFlagBits::eNone,
                vk::AccessFlagBits::eDepthStencilAttachmentWrite,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eEarlyFragmentTests,
                vk::ImageAspectFlagBits::eDepth
            );
        }
    );

    for(uint32_t i = 0; i < 3; i++)
    {
        vk::CommandBuffer commandBuffer = 
            make_command_buffer(m_logicalDevice, m_commandPool, m_renderDeletionQueue);

        m_frames.push_back(Frame(
            &m_swapchain, m_logicalDevice, &m_shaders, commandBuffer, 
            m_graphicsInterface.get_descriptor_set_layouts().data(), 
            &m_descriptorPool, &m_pipelineLayout, &m_depthImage,
            &m_mesh, m_renderDeletionQueue
        ));
    }

    m_imagesInFlight.resize(m_swapchain.imageViews.size(), VK_NULL_HANDLE);

    return true;
}

void Engine::destroy_render_resources()
{
    if(m_graphicsQueue)
        (void) m_graphicsQueue.waitIdle();

    while(!m_renderDeletionQueue.empty())
    {
        if(m_logicalDevice)
            (m_renderDeletionQueue.back())(m_logicalDevice);
        
        m_renderDeletionQueue.pop_back();
    }

    while(!m_renderVmaDeletionQueue.empty())
    {
        if(m_allocator)
            (m_renderVmaDeletionQueue.back())(m_allocator);
        
        m_renderVmaDeletionQueue.pop_back();
    }

    m_frames.clear();
    m_imagesInFlight.clear();
}

void Engine::shutdown()
{
    if(!m_initialized) return;

    m_logger->set_mode(true);

    if(m_graphicsQueue)
        (void) m_graphicsQueue.waitIdle();

    destroy_render_resources();
    
    while(!m_vmaDeletionQueue.empty())
    {
        if(m_allocator)
            (m_vmaDeletionQueue.back())(m_allocator);
        
        m_vmaDeletionQueue.pop_back();
    }

    if(m_allocator)
        vmaDestroyAllocator(m_allocator);

    while(!m_deviceDeletionQueue.empty())
    {
        if(m_logicalDevice)
            (m_deviceDeletionQueue.back())(m_logicalDevice);
        
        m_deviceDeletionQueue.pop_back();
    }
    
    while(!m_instanceDeletionQueue.empty())
    {
        if(m_instance)
            (m_instanceDeletionQueue.back())(m_instance);

        m_instanceDeletionQueue.pop_back();
    }

    m_initialized = false;
}

Engine::~Engine()
{
    shutdown();
    m_logger->print("Deleted graphics engine");
}
