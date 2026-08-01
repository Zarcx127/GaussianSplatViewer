/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

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

    vk::PhysicalDevice physical_device() const;
    
    vk::Device logical_device() const;
    vk::Device& logical_device_ref();

    VmaAllocator allocator() const;

    vk::SurfaceKHR surface() const;

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
