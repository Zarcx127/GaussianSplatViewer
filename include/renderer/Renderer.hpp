#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#define GLFW_INCLUDE_VULKAN

#include <deque> 
#include <functional>

#include <GLFW/glfw3.h>
#include <vma/vk_mem_alloc.h>

#include <vulkan/vulkan.hpp>

#include "logging/Logger.hpp"

#include "renderer/core/Frame.hpp"
#include "renderer/core/Swapchain.hpp"

class Engine
{
public:
    bool rebuildSwapchain { false };

    Engine(GLFWwindow* window);
    
    void draw();
    void update_timing();
    
    ~Engine();
    
private:
    GLFWwindow* m_window { nullptr };
    Logger* m_logger { nullptr };
    
    std::deque<std::function<void(vk::Instance)>> m_instanceDeletionQueue;
    std::deque<std::function<void(vk::Device)>> m_deviceDeletionQueue;
    std::deque<std::function<void(vk::Device)>> m_renderDeletionQueue;
    std::deque<std::function<void(VmaAllocator)>> m_vmaDeletionQueue;
    
    vk::Instance m_instance;

    vk::DebugUtilsMessengerEXT m_debugMessenger;
 
    vk::PhysicalDevice m_physicalDevice;
    vk::Device m_logicalDevice;

    VmaAllocator m_allocator;
     
    uint32_t m_graphicsQueueFamilyIndex;
    vk::Queue m_graphicsQueue;

    vk::SurfaceKHR m_surface;

    Swapchain m_swapchain;
    std::vector<Frame> m_frames;
    std::vector<vk::ShaderEXT> m_shaders;

    Mesh m_mesh;

    vk::CommandPool m_commandPool;
    vk::DescriptorSetLayout m_descriptorSetLayout;
    vk::PipelineLayout m_pipelineLayout;

    std::vector<vk::Fence> m_imagesInFlight;

    double m_currentTime;
    double m_lastTime;
    double m_frameTime;

    uint32_t m_currFrame { 0 };
    uint32_t m_numFrames { 0 };

    void destroy_render_resources();
    void init_render_resources();

    void clean_up();
};

#endif
