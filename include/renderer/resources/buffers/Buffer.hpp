#pragma once

#ifndef RENDERER_RESOURCES_BUFFERS_BUFFER_H
#define RENDERER_RESOURCES_BUFFERS_BUFFER_H

#include <vulkan/vulkan.hpp>

#include <vma/vk_mem_alloc.h>

struct AllocatedBuffer
{
    vk::Buffer buffer {};
    
    VmaAllocation allocation { nullptr };
    VmaAllocationInfo allocationInfo {};

    vk::DeviceSize size { 0 };
};

bool copy_buffer(
    vk::Buffer srcBuffer,
    vk::Buffer dstBuffer,
    vk::DeviceSize copySize,
    vk::Device device,
    vk::Queue queue,
    vk::CommandPool commandPool
);

AllocatedBuffer create_buffer(
    VmaAllocator allocator,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags allocationFlags,
    const char* allocationName
);

bool write_buffer(
    VmaAllocator allocator,
    const AllocatedBuffer& buffer,
    const void* data,
    vk::DeviceSize size
);

void destroy_buffer(VmaAllocator allocator, AllocatedBuffer& buffer);

#endif
