#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.hpp>

#include <vma/vk_mem_alloc.h>

void copy_buffer(
    vk::Buffer srcBuffer,
    VmaAllocationInfo srcInfo,
    vk::Buffer dstBuffer,
    VmaAllocationInfo dstInfo,
    vk::DeviceSize copySize,
    vk::Device device,
    vk::Queue queue,
    vk::CommandPool commandPool
);

#endif
