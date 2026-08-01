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
