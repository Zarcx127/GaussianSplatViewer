#include "renderer/Renderer.hpp"

#ifdef RENDERER_H

#include "factory/MeshFactory.hpp"

#include "renderer/core/Device.hpp"
#include "renderer/core/Command.hpp"
#include "renderer/core/Instance.hpp"
#include "renderer/core/Swapchain.hpp"
#include "renderer/core/Synchronization.hpp"

#include "renderer/resources/Shader.hpp"
#include "renderer/resources/Allocator.hpp"
#include "renderer/resources/Descriptors.hpp"

Engine::Engine(GLFWwindow* window)
{
    m_logger = Logger::get_logger();
    m_window = window;

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    
    m_instance = make_instance("VK Engine", m_instanceDeletionQueue);
    if(!m_instance)
    {
        clean_up();
        return;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

    m_debugMessenger = m_logger->make_debug_messenger(m_instance, m_instanceDeletionQueue);
    if(!m_debugMessenger && m_logger->is_enabled())
    {
        clean_up();
        return;
    }

    VkSurfaceKHR rawSurface;
    VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &rawSurface);
    if (result != VK_SUCCESS)
    {
        m_logger->print("Failed to create window surface");
        
        clean_up();
        return;
    }

    m_surface = vk::SurfaceKHR(rawSurface);
    m_instanceDeletionQueue.push_back([this] (vk::Instance instance)->void{
        instance.destroySurfaceKHR(m_surface);
        m_logger->print("Deleted Vulkan surface");
    });

    m_logger->set_mode(false);
    m_physicalDevice = choose_physical_device(m_instance);
    if(!m_physicalDevice)
    {
        clean_up();
        return;
    }
    
    m_logicalDevice = create_logical_device(m_physicalDevice, m_surface, m_deviceDeletionQueue);
    if(!m_logicalDevice)
    {
        clean_up();
        return;
    }

    m_graphicsQueueFamilyIndex = findQueueFamilyIndex(
        m_physicalDevice, m_surface, vk::QueueFlagBits::eGraphics 
    );

    if(m_graphicsQueueFamilyIndex == UINT32_MAX)
    {
        m_logger->print("No graphics queue found");

        clean_up();
        return;
    }

    m_graphicsQueue = m_logicalDevice.getQueue(m_graphicsQueueFamilyIndex, 0);

    m_commandPool = make_command_pool(m_logicalDevice, m_graphicsQueueFamilyIndex, m_deviceDeletionQueue);
    if(!m_commandPool)
    {
        clean_up();
        return;
    }

    m_allocator = make_vma_allocator(m_instance, m_physicalDevice, m_logicalDevice);

    m_logger->set_mode(true);
    m_mesh = build_triangle(m_allocator, m_logicalDevice, m_commandPool, m_graphicsQueue, m_vmaDeletionQueue);
    m_logger->set_mode(false);

    m_frames.reserve(3);
    init_render_resources();

    m_currentTime = glfwGetTime();
    m_lastTime = m_currentTime;
    m_numFrames = 0;

    m_logger->print("Graphics engine started");
}

void Engine::draw()
{
    if(rebuildSwapchain)
    {
        (void) m_logicalDevice.waitIdle();
        destroy_render_resources();
        init_render_resources();

        m_currFrame = 0;
        rebuildSwapchain = false;

        return;
    }

    Frame& frame = m_frames[m_currFrame];

    (void) m_logicalDevice.waitForFences(frame.renderFinishedFence, VK_FALSE, UINT64_MAX);
    
    vk::ResultValue<uint32_t> imageIndexAttempt = m_logicalDevice.acquireNextImageKHR(
        m_swapchain.chain, UINT64_MAX, frame.imageAquiredSemaphore
    );

    if(
        (imageIndexAttempt.result == vk::Result::eErrorOutOfDateKHR) 
        || (imageIndexAttempt.result == vk::Result::eSuboptimalKHR)
    ) {
        rebuildSwapchain = true;
        return;
    }

    if(imageIndexAttempt.result != vk::Result::eSuccess) return;

    uint32_t imageIndex = imageIndexAttempt.value;
    if(m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        (void) m_logicalDevice.waitForFences(1, &(m_imagesInFlight[imageIndex]), VK_TRUE, UINT64_MAX);

    m_imagesInFlight[imageIndex] = frame.renderFinishedFence;
    (void) m_logicalDevice.resetFences(frame.renderFinishedFence);
    
    frame.record_command_buffer(imageIndex);
    
    vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submitInfo = {};

    submitInfo.commandBufferCount = 1;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.signalSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &(frame.commandBuffer);
    submitInfo.pWaitSemaphores = &(frame.imageAquiredSemaphore);
    submitInfo.pSignalSemaphores = &(frame.renderFinishedSemaphores[imageIndex]);
    submitInfo.pWaitDstStageMask = &waitStage;

    (void) m_graphicsQueue.submit(submitInfo, frame.renderFinishedFence);

    vk::PresentInfoKHR presentInfo = {};
    
    presentInfo.swapchainCount = 1;
    presentInfo.waitSemaphoreCount = 1;
    
    presentInfo.pSwapchains = &(m_swapchain.chain);
    presentInfo.pWaitSemaphores = &(frame.renderFinishedSemaphores[imageIndex]);
    presentInfo.pImageIndices = &imageIndex;
    
    vk::Result presentResult = m_graphicsQueue.presentKHR(presentInfo);
    if(
        (presentResult == vk::Result::eErrorOutOfDateKHR)
        || (presentResult == vk::Result::eSuboptimalKHR)
    ) {
        rebuildSwapchain = true;
        return;
    }

    m_currFrame = (m_currFrame + 1) % m_frames.size();
}

void Engine::update_timing()
{
    m_currentTime = glfwGetTime();
    double delta = m_currentTime - m_lastTime;

    if(delta >= 1)
    {
        uint32_t frameRate = std::max(1U, uint32_t(m_numFrames / delta));

        std::stringstream title;
        title << "Render Thread running at " << frameRate << " fps";

        glfwSetWindowTitle(m_window, title.str().c_str());

        m_lastTime = m_currentTime;
        
        m_numFrames = -1;
        m_frameTime = 1000.0 / frameRate;
    }

    m_numFrames++;
}

void Engine::init_render_resources()
{
    m_logger->set_mode(false);

    int intWidth, intHeight;
    glfwGetWindowSize(m_window, &intWidth, &intHeight);

    uint32_t width = static_cast<uint32_t>(intWidth);
    uint32_t height = static_cast<uint32_t>(intHeight);

    if((width == 0) || (height == 0)) return;

    m_swapchain.build(m_logicalDevice, m_physicalDevice, m_surface, width, height, m_renderDeletionQueue);
    if(m_swapchain.imageViews.empty())
    {
        m_logger->print("Rendering crashed");
        
        clean_up();
        return;
    }

    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder(m_logicalDevice);
    descriptorSetLayoutBuilder.add_entry(vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageImage);

    m_descriptorSetLayout = descriptorSetLayoutBuilder.build(m_renderDeletionQueue);
    if(!m_descriptorSetLayout)
    {
        m_logger->print("Rendering crashed");
        
        clean_up();
        return;
    }

    PipelineLayoutBuilder pipelineLayoutBuilder(m_logicalDevice);
    pipelineLayoutBuilder.add(m_descriptorSetLayout);

    m_pipelineLayout = pipelineLayoutBuilder.build(m_renderDeletionQueue);
    if(!m_pipelineLayout)
    {
        m_logger->print("Rendering crashed");
        
        clean_up();
        return;
    }

    m_shaders = make_shader_object(
        m_logicalDevice, "shaders/bin/shader.vert.spv", "shaders/bin/shader.frag.spv", m_deviceDeletionQueue
    );

    if(m_shaders.empty())
    {
        clean_up();
        return;
    }

    for(uint32_t i = 0; i < 3; i++)
    {
        vk::CommandBuffer commandBuffer = allocate_command_buffer(m_logicalDevice, m_commandPool);
        m_frames.push_back(Frame(m_swapchain, m_logicalDevice, m_shaders, commandBuffer, &m_mesh, m_deviceDeletionQueue));
    }

    m_imagesInFlight.resize(m_swapchain.imageViews.size(), VK_NULL_HANDLE);

    m_logger->set_mode(true);
}

void Engine::destroy_render_resources()
{
    (void) m_graphicsQueue.waitIdle();

    while(!m_renderDeletionQueue.empty())
    {
        if(m_logicalDevice)
            (m_renderDeletionQueue.back())(m_logicalDevice);
        
        m_renderDeletionQueue.pop_back();
    }

    m_frames.clear();
    m_imagesInFlight.clear();
}

void Engine::clean_up()
{
    (void) m_graphicsQueue.waitIdle();
    
    while(!m_vmaDeletionQueue.empty())
    {
        if(m_allocator)
            (m_vmaDeletionQueue.back())(m_allocator);
        
        m_vmaDeletionQueue.pop_back();
    }

    vmaDestroyAllocator(m_allocator);
    destroy_render_resources();

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
}

Engine::~Engine()
{
    clean_up();
    m_logger->print("Deleted graphics engine");
}

#endif
