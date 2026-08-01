#pragma once

#ifndef RENDERER_CORE_FRAME_H
#define RENDERER_CORE_FRAME_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "renderer/resources/splats/SplatFrame.hpp"

class Frame
{
public:
    vk::CommandBuffer commandBuffer;
    SplatFrameResources splatResources;

    vk::DescriptorSet splatFrameDescriptorSet;

    vk::Semaphore imageAcquiredSemaphore;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    vk::Fence renderFinishedFence;
    
    Frame(
        vk::Device device, 
        vk::CommandBuffer commandBuffer,
        const SplatFrameResources& splatResources,
        vk::DescriptorSet splatFrameDescriptorSet,
        uint32_t swapchainImageCount,
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;

    Frame(Frame&&) noexcept = default;
    Frame& operator=(Frame&&) noexcept = default;

    bool is_valid(uint32_t swapchainImageCount) const;
};

#endif
