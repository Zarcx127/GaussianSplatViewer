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

#include "renderer/core/Frame.hpp"

#include "renderer/core/Synchronization.hpp"

Frame::Frame(
    vk::Device device, 
    vk::CommandBuffer commandBuffer,
    const SplatFrameResources& splatResources,
    vk::DescriptorSet splatFrameDescriptorSet,
    uint32_t swapchainImageCount,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    this->commandBuffer = commandBuffer;
    this->splatResources = splatResources;
    this->splatFrameDescriptorSet = splatFrameDescriptorSet;

    imageAcquiredSemaphore = make_semaphore(device, deletionQueue);

    renderFinishedSemaphores.resize(swapchainImageCount);
    for(vk::Semaphore& renderFinishedSemaphore : renderFinishedSemaphores)
        renderFinishedSemaphore = make_semaphore(device, deletionQueue);

    renderFinishedFence = make_fence(device, deletionQueue);
}

bool Frame::is_valid(uint32_t swapchainImageCount) const
{
    if(
        !commandBuffer ||
        !splatFrameDescriptorSet ||
        !imageAcquiredSemaphore ||
        !renderFinishedFence ||
        (renderFinishedSemaphores.size() != swapchainImageCount) ||
        !splat_frame_resources_are_valid(splatResources)
    ) {
        return false;
    }

    for(const vk::Semaphore& semaphore : renderFinishedSemaphores)
        if(!semaphore) 
            return false;

    return true;
}
