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
    deviceDeletionQueue.push_back([semaphore, logger] (vk::Device device)->void{
        (void) device.waitIdle();
        device.destroySemaphore(semaphore);

        logger->print("Deleted semaphore");
    });

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
    deviceDeletionQueue.push_back([fence, logger] (vk::Device device)->void{
        (void) device.waitIdle();
        device.destroyFence(fence);
        
        logger->print("Deleted fence");
    });

    return fence;
}
