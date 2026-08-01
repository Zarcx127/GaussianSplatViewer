#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#define GLFW_INCLUDE_VULKAN

#include <deque> 
#include <functional>

#include <GLFW/glfw3.h>
#include <vma/vk_mem_alloc.h>   

#include <vulkan/vulkan.hpp>

#include "AppState.hpp"

#include "logging/Logger.hpp"

#include "renderer/core/Frame.hpp"
#include "renderer/core/Swapchain.hpp"

#include "renderer/resources/images/AllocatedImage.hpp"
#include "renderer/resources/shaders/ShaderInterface.hpp"

class Engine
{
public:
    Engine(GLFWwindow* window);

    void render_loop(AppState& state);
    
    ~Engine();
    
private:
    GLFWwindow* m_window { nullptr };
    Logger* m_logger { nullptr };
    
    std::deque<std::function<void(vk::Instance)>> m_instanceDeletionQueue;
    std::deque<std::function<void(vk::Device)>> m_deviceDeletionQueue;
    std::deque<std::function<void(vk::Device)>> m_renderDeletionQueue;

    std::deque<std::function<void(VmaAllocator)>> m_vmaDeletionQueue;
    std::deque<std::function<void(VmaAllocator)>> m_renderVmaDeletionQueue;
    
    vk::Instance m_instance;

    vk::DebugUtilsMessengerEXT m_debugMessenger;
 
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_logicalDevice;

    VmaAllocator m_allocator { nullptr };
     
    uint32_t m_graphicsQueueFamilyIndex;
    vk::Queue m_graphicsQueue;

    vk::SurfaceKHR m_surface;

    std::vector<vk::ShaderEXT> m_shaders;
    vk::ShaderEXT m_computeShader;

    Mesh m_mesh;

    ShaderInterface m_graphicsInterface;

    vk::DescriptorPool m_descriptorPool;
    vk::PipelineLayout m_pipelineLayout;

    vk::CommandPool m_commandPool;

    Swapchain m_swapchain;
    AllocatedImage m_depthImage;
    std::vector<Frame> m_frames;

    std::vector<vk::Fence> m_imagesInFlight;

    double m_currentTime;
    double m_lastTime;
    double m_frameTime;

    uint32_t m_currFrame { 0 };
    uint32_t m_numFrames { 0 };

    bool m_rebuildSwapchain { false };
    bool m_initialized { false };

    enum class DrawResult
    {
        Success,
        Skipped,
        NeedsSwapchainRebuild,
        FatalError
    };

    bool init(uint32_t width, uint32_t height);

    DrawResult draw(uint32_t width, uint32_t height);
    void update_timing(AppState& state);

    bool init_render_resources(uint32_t width, uint32_t height);
    void destroy_render_resources();

    void shutdown();
};

#endif
