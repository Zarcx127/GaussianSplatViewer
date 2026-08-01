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
