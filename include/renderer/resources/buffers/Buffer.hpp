#pragma once

#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.hpp>

#include <vma/vk_mem_alloc.h>

void copy_buffer(
    vk::Buffer srcBuffer,
    vk::Buffer dstBuffer,
    vk::DeviceSize copySize,
    vk::Device device,
    vk::Queue queue,
    vk::CommandPool commandPool
);

#endif
