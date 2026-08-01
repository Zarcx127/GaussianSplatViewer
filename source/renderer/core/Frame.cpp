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
        !splatResources.projectedSplats.buffer ||
        !splatResources.visibleSplatIndices.buffer ||
        !splatResources.sortKeys[0].buffer ||
        !splatResources.sortKeys[1].buffer ||
        !splatResources.entrySplatIndices[0].buffer ||
        !splatResources.entrySplatIndices[1].buffer ||
        !splatResources.counters.buffer ||
        !splatResources.drawCommand.buffer ||
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
