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
