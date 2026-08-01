#pragma once

#ifndef RENDERER_CORE_SYNCHRONIZATION_H
#define RENDERER_CORE_SYNCHRONIZATION_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

vk::Semaphore make_semaphore(
    vk::Device device, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::Fence make_fence(
    vk::Device device, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::BufferMemoryBarrier make_buffer_memory_barrier(
    vk::Buffer buffer,
    vk::DeviceSize size,
    vk::AccessFlags sourceAccess,
    vk::AccessFlags destinationAccess
);

#endif
