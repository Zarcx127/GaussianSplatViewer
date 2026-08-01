#include "renderer/core/Frame.hpp"

#include "renderer/core/Synchronization.hpp"

Frame::Frame(
    vk::Device device, 
    vk::CommandBuffer commandBuffer,
    uint32_t swapchainImageCount,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    this->commandBuffer = commandBuffer;

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
        !imageAcquiredSemaphore ||
        !renderFinishedFence ||
        (renderFinishedSemaphores.size() != swapchainImageCount)
    ) {
        return false;
    }

    for(const vk::Semaphore& semaphore : renderFinishedSemaphores)
        if(!semaphore) 
            return false;

    return true;
}
