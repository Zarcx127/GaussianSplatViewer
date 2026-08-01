#include "factory/SplatFrameFactory.hpp"

#include <vulkan/vulkan.hpp>

namespace
{
    AllocatedBuffer create_processing_buffer(
        VmaAllocator allocator,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        const char* name
    );

    bool is_valid(const SplatFrameResources& resources);

    void destroy_splat_frame_resources(
        VmaAllocator allocator,
        SplatFrameResources& resources
    );
}

SplatFrameResources build_splat_frame_resources(
    VmaAllocator allocator,
    uint32_t splatCapacity,
    uint32_t entryCapacity,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
) {
    if(!allocator || (splatCapacity == 0) || (entryCapacity == 0))
        return {};

    SplatFrameResources resources = {};

    resources.splatCapacity = splatCapacity;
    resources.entryCapacity = entryCapacity;

    resources.projectedSplats = create_processing_buffer(
        allocator,
        (sizeof(GpuProjectedSplat) * static_cast<vk::DeviceSize>(splatCapacity)),
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Projected Splat Buffer"
    );

    resources.visibleSplatIndices = create_processing_buffer(
        allocator,
        (sizeof(uint32_t) * static_cast<vk::DeviceSize>(splatCapacity)),
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Visible Splat Index Buffer"
    );

    const vk::DeviceSize KEY_BUFFER_SIZE = (
        (sizeof(GpuSplatSortKey) * static_cast<vk::DeviceSize>(entryCapacity))
    );

    const vk::DeviceSize INDEX_BUFFER_SIZE = (
        (sizeof(uint32_t) * static_cast<vk::DeviceSize>(entryCapacity))
    );

    resources.sortKeys[0] = create_processing_buffer(
        allocator,
        KEY_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Sort Key Buffer A"
    );

    resources.sortKeys[1] = create_processing_buffer(
        allocator,
        KEY_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Sort Key Buffer B"
    );

    resources.entrySplatIndices[0] = create_processing_buffer(
        allocator,
        INDEX_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Entry Splat Index Key Buffer A"
    );

    resources.entrySplatIndices[1] = create_processing_buffer(
        allocator,
        INDEX_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Entry Splat Index Key Buffer B"
    );

    resources.counters = create_processing_buffer(
        allocator,
        sizeof(GpuSplatCounters),
        (
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eTransferDst 
        ),
        "Splat Counter Buffer"
    );

    resources.drawCommand = create_processing_buffer(
        allocator,
        sizeof(GpuSplatDrawCommand),
        (
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eIndirectBuffer |
            vk::BufferUsageFlagBits::eTransferDst
        ),
        "Splat Draw Command Buffer"
    );

    if(!is_valid(resources))
    {
        destroy_splat_frame_resources(allocator, resources);
        return {};
    }

    deletionQueue.push_back(
        [resources] (VmaAllocator allocator) mutable->void {
            destroy_splat_frame_resources(allocator, resources);
        }
    );

    return resources;
}

namespace
{
    AllocatedBuffer create_processing_buffer(
        VmaAllocator allocator,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        const char* name
    ) {
        return create_buffer(
            allocator,
            size,
            usage,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            0,
            name
        );
    }

    bool is_valid(const SplatFrameResources& resources)
    {
        return (
            resources.projectedSplats.buffer &&
            resources.visibleSplatIndices.buffer &&
            resources.sortKeys[0].buffer &&
            resources.sortKeys[1].buffer &&
            resources.entrySplatIndices[0].buffer &&
            resources.entrySplatIndices[1].buffer &&
            resources.counters.buffer &&
            resources.drawCommand.buffer
        );
    }

    void destroy_splat_frame_resources(
        VmaAllocator allocator,
        SplatFrameResources& resources
    ) {
        destroy_buffer(allocator, resources.projectedSplats);
        destroy_buffer(allocator, resources.visibleSplatIndices);

        for(AllocatedBuffer& buffer : resources.sortKeys)
            destroy_buffer(allocator, buffer);
        
        for(AllocatedBuffer& buffer : resources.entrySplatIndices)
            destroy_buffer(allocator, buffer);
    
        destroy_buffer(allocator, resources.counters);
        destroy_buffer(allocator, resources.drawCommand);

        resources = {};
    }
}
