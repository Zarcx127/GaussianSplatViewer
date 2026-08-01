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
