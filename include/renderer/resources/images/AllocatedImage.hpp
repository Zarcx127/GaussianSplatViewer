#pragma once

#ifndef RENDERER_RESOURCES_IMAGES_ALLOCATED_IMAGE_H
#define RENDERER_RESOURCES_IMAGES_ALLOCATED_IMAGE_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

struct AllocatedImage
{
    vk::Image image {};
    vk::ImageView imageView {};
    
    VmaAllocation allocation { VK_NULL_HANDLE };

    vk::Format format { vk::Format::eUndefined };
    vk::Extent2D extent {};
};

vk::Format find_depth_format(vk::PhysicalDevice physicalDevice);

AllocatedImage create_depth_image(
    VmaAllocator allocator,
    vk::Device logicalDevice,
    vk::PhysicalDevice physicalDevice,
    vk::Extent2D extent,
    std::deque<std::function<void(vk::Device)>>& renderDeletionQueue,
    std::deque<std::function<void(VmaAllocator)>>& renderVmaDeletionQueue
);

#endif
