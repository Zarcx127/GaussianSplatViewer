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

#include "renderer/core/Synchronization.hpp"

#include "logging/Logger.hpp"

vk::Semaphore make_semaphore(
    vk::Device device, 
    std::deque<std::function<void(vk::Device)>>& deviceDeletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::ResultValue<vk::Semaphore> semaphoreAttempt = device.createSemaphore({});
    if(semaphoreAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create semaphore");
        return vk::Semaphore();
    }

    vk::Semaphore semaphore = semaphoreAttempt.value;
    deviceDeletionQueue.push_back(
        [logger, semaphore] (vk::Device device)->void {
            device.destroySemaphore(semaphore);
            logger->print("Deleted semaphore");
        }
    );

    return semaphore;
}

vk::Fence make_fence(
    vk::Device device, 
    std::deque<std::function<void(vk::Device)>>& deviceDeletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::FenceCreateInfo createInfo = {};
    createInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    vk::ResultValue<vk::Fence> fenceAttempt = device.createFence(createInfo);
    if(fenceAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create fence");
        return vk::Fence();
    }

    vk::Fence fence = fenceAttempt.value;
    deviceDeletionQueue.push_back(
        [logger, fence] (vk::Device device)->void {
            device.destroyFence(fence);
            logger->print("Deleted fence");
        }
    );

    return fence;
}

vk::BufferMemoryBarrier make_buffer_memory_barrier(
    vk::Buffer buffer,
    vk::DeviceSize size,
    vk::AccessFlags sourceAccess,
    vk::AccessFlags destinationAccess
) {
    vk::BufferMemoryBarrier barrier = {};

    barrier.srcAccessMask = sourceAccess;
    barrier.dstAccessMask = destinationAccess;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.buffer = buffer;
    barrier.offset = 0;
    barrier.size = size;

    return barrier;
}
