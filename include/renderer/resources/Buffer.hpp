#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.hpp>

#include <vma/vk_mem_alloc.h>

void copyBuffer(
    vk::Buffer srcBuffer,
    VmaAllocationInfo srcInfo,
    vk::Buffer dstBuffer,
    VmaAllocationInfo dstInfo,
    uint32_t copySize,
    vk::Device device,
    vk::Queue queue,
    vk::CommandPool commandPool
);

#endif
