#pragma once

#ifndef RENDERER_CORE_VULKAN_CONTEXT_H
#define RENDERER_CORE_VULKAN_CONTEXT_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include <GLFW/glfw3.h>

class VulkanContext
{
public:
    VulkanContext() = default;

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    VulkanContext(VulkanContext&&) = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    bool build(GLFWwindow* window, const char* applicationName);
    void destroy();

    vk::Instance instance() const;

    vk::PhysicalDevice physical_device() const;
    
    vk::Device logical_device() const;
    vk::Device& logical_device_ref();

    VmaAllocator allocator() const;

    vk::SurfaceKHR surface() const;

    uint32_t graphics_queue_family_index() const;
    vk::Queue graphics_queue() const;

    vk::CommandPool command_pool() const;

private:
    std::deque<std::function<void(vk::Instance)>> m_instanceDeletionQueue;
    std::deque<std::function<void(vk::Device)>> m_deviceDeletionQueue;
    
    vk::Instance m_instance {};

    vk::DebugUtilsMessengerEXT m_debugMessenger {};
 
    vk::PhysicalDevice m_physicalDevice {};
    vk::Device m_logicalDevice {};

    VmaAllocator m_allocator { nullptr };
     
    uint32_t m_graphicsQueueFamilyIndex { UINT32_MAX };
    vk::Queue m_graphicsQueue {};

    vk::SurfaceKHR m_surface {};

    vk::CommandPool m_commandPool {};
};

#endif
