#pragma once

#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

vk::Semaphore make_semaphore(
    vk::Device device, std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::Fence make_fence(
    vk::Device device, std::deque<std::function<void(vk::Device)>>& deletionQueue
);

#endif
