#include "renderer/Renderer.hpp"

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
}

bool Engine::init()
{
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    
    m_instance = make_instance("VK Engine", m_instanceDeletionQueue);
    if(!m_instance)
    {
        clean_up();
        return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

#ifdef DEBUG
    
    m_debugMessenger = m_logger->make_debug_messenger(m_instance, m_instanceDeletionQueue);
    if(!m_debugMessenger && m_logger->is_enabled())
    {
        clean_up();
        return false;
    }

#endif

    VkSurfaceKHR rawSurface;
    VkResult rawSurfaceResult = glfwCreateWindowSurface(m_instance, m_window, nullptr, &rawSurface);
    if(rawSurfaceResult != VK_SUCCESS)
    {
        m_logger->print("Failed to create window surface");
        
        clean_up();
        return false;
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
        return false;
    }
    
    m_logicalDevice = create_logical_device(m_physicalDevice, m_surface, m_deviceDeletionQueue);
    if(!m_logicalDevice)
    {
        clean_up();
        return false;
    }

    m_graphicsQueueFamilyIndex = find_queue_family_index(
        m_physicalDevice, m_surface, vk::QueueFlagBits::eGraphics 
    );

    if(m_graphicsQueueFamilyIndex == UINT32_MAX)
    {
        m_logger->print("No graphics queue found");

        clean_up();
        return false;
    }

    m_graphicsQueue = m_logicalDevice.getQueue(m_graphicsQueueFamilyIndex, 0);

    m_commandPool = make_command_pool(m_logicalDevice, m_graphicsQueueFamilyIndex, m_deviceDeletionQueue);
    if(!m_commandPool)
    {
        clean_up();
        return false;
    }

    m_allocator = make_vma_allocator(m_instance, m_physicalDevice, m_logicalDevice);
    if(!m_allocator)
    {
        clean_up();
        return false;
    }

    m_mesh = build_cube(m_allocator, m_logicalDevice, m_commandPool, m_graphicsQueue, m_vmaDeletionQueue);

/////
// future use
/////

    DescriptorSetLayoutBuilder descriptorSetLayoutBuilder(m_logicalDevice);
    descriptorSetLayoutBuilder.add_entry(vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageImage);

    m_descriptorSetLayout = descriptorSetLayoutBuilder.build(m_deviceDeletionQueue);
    if(!m_descriptorSetLayout)
    {
        clean_up();
        return false;
    }

    PipelineLayoutBuilder pipelineLayoutBuilder(m_logicalDevice);
    pipelineLayoutBuilder.add(m_descriptorSetLayout);
    pipelineLayoutBuilder.add_push_constant_range(vk::ShaderStageFlagBits::eVertex, sizeof(glm::mat4));

    m_pipelineLayout = pipelineLayoutBuilder.build(m_deviceDeletionQueue);
    if(!m_pipelineLayout)
    {
        clean_up();
        return false;
    }

////
////

    m_shader = make_compute_shader(
        m_logicalDevice, "shaders/bin/clear_screen.comp.spv", &m_descriptorSetLayout, m_deviceDeletionQueue
    );

    if(!m_shader)
    {
        clean_up();
        return false;
    }

    m_shaders = make_shader_object(
        m_logicalDevice, "shaders/bin/shader.vert.spv", "shaders/bin/shader.frag.spv", m_deviceDeletionQueue
    );

    if(m_shaders.empty())
    {
        clean_up();
        return false;
    }

    m_frames.reserve(3);
    if(!init_render_resources())
    {
        clean_up();
        return false;
    }

    m_currentTime = glfwGetTime();
    m_lastTime = m_currentTime;
    m_numFrames = 0;

    m_logger->print("Graphics engine started");
    return true;
}

void Engine::draw()
{
    if(rebuildSwapchain)
    {
        (void) m_logicalDevice.waitIdle();

        destroy_render_resources();
        if(!init_render_resources())
            return;

        m_currFrame = 0;
        rebuildSwapchain = false;
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
    
    float aspect = 
        static_cast<float>(m_swapchain.extent.width) /
        static_cast<float>(m_swapchain.extent.height);

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
    m_numFrames++;

    m_currentTime = glfwGetTime();
    double delta = m_currentTime - m_lastTime;

    if(delta >= 1)
    {
        uint32_t frameRate = std::max(1U, uint32_t(m_numFrames / delta));

        std::stringstream title;
        title << "Render Thread running at " << frameRate << " fps";

        glfwSetWindowTitle(m_window, title.str().c_str());

        m_lastTime = m_currentTime;
        
        m_numFrames = 0;
        m_frameTime = 1000.0 / frameRate;
    }
}

bool Engine::init_render_resources()
{
    int intWidth, intHeight;
    glfwGetWindowSize(m_window, &intWidth, &intHeight);

    uint32_t width = static_cast<uint32_t>(intWidth);
    uint32_t height = static_cast<uint32_t>(intHeight);

    if((width == 0) || (height == 0)) 
        return false;

/////
// future use
/////

    DescriptorPoolBuilder descriptorPoolBuilder(m_logicalDevice);
    descriptorPoolBuilder.add_entry(vk::DescriptorType::eStorageImage);

    m_descriptorPool = descriptorPoolBuilder.build(3, m_renderDeletionQueue);
    if(!m_descriptorPool)
    {
        m_logger->print("Rendering crashed");
        destroy_render_resources();
        
        return false;
    }

////
////

    m_swapchain.build(m_logicalDevice, m_physicalDevice, m_surface, width, height, m_renderDeletionQueue);
    if(m_swapchain.imageViews.empty())
    {
        m_logger->print("Rendering crashed");
        destroy_render_resources();

        return false;
    }

    for(uint32_t i = 0; i < 3; i++)
    {
        vk::CommandBuffer commandBuffer = allocate_command_buffer(m_logicalDevice, m_commandPool);

        m_frames.push_back(Frame(
            &m_swapchain, m_logicalDevice, &m_shaders, commandBuffer, &m_descriptorSetLayout, 
            &m_descriptorPool, &m_pipelineLayout, &m_mesh, m_renderDeletionQueue
        ));
    }

    m_imagesInFlight.resize(m_swapchain.imageViews.size(), VK_NULL_HANDLE);

    return true;
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
    m_logger->set_mode(true);

    if(m_graphicsQueue)
        (void) m_graphicsQueue.waitIdle();
    
    while(!m_vmaDeletionQueue.empty())
    {
        if(m_allocator)
            (m_vmaDeletionQueue.back())(m_allocator);
        
        m_vmaDeletionQueue.pop_back();
    }

    if(m_allocator)
    {
        vmaDestroyAllocator(m_allocator);
        destroy_render_resources();
    }

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
