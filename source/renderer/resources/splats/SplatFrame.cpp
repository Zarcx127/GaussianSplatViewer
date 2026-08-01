#include "renderer/resources/splats/SplatFrame.hpp"

#include <cstring>

bool read_splat_frame_counters(
    VmaAllocator allocator,
    const SplatFrameResources& resources,
    GpuSplatCounters& counters    
) {
    if(
        !allocator ||
        !resources.counterReadback.buffer ||
        !resources.counterReadback.allocation ||
        (resources.counterReadback.size < sizeof(GpuSplatCounters))
    ) {
        return false;
    }

    void* mappedData = nullptr;
    VkResult mapMemoryAttempt = vmaMapMemory(
        allocator,
        resources.counterReadback.allocation,
        &mappedData
    ); 
    
    if(mapMemoryAttempt != VK_SUCCESS)
        return false;

    VkResult invalidateMemoryAttempt = vmaInvalidateAllocation(
        allocator,
        resources.counterReadback.allocation,
        0,
        sizeof(GpuSplatCounters)
    );

    if(invalidateMemoryAttempt != VK_SUCCESS)
    {
        vmaUnmapMemory(allocator, resources.counterReadback.allocation);
        return false;
    }

    std::memcpy(
        &counters, mappedData,
        sizeof(GpuSplatCounters)
    );

    vmaUnmapMemory(allocator, resources.counterReadback.allocation);

    return true;
}

bool splat_frame_resources_are_valid(
    const SplatFrameResources& resources
) {
    return (
        resources.projectedSplats.buffer &&
        resources.visibleSplatIndices.buffer &&
        resources.entryRanges.buffer &&
        resources.entryKeys[0].buffer &&
        resources.entryKeys[1].buffer &&
        resources.entrySplatIndices[0].buffer &&
        resources.entrySplatIndices[1].buffer &&
        resources.counters.buffer &&
        resources.counterReadback.buffer &&
        resources.drawCommand.buffer &&
        resources.tileRanges.buffer &&
        resources.entryScanBlockSums.buffer &&
        (resources.splatCapacity > 0) &&
        (resources.entryCapacity > 0) &&
        (resources.tileCapacity > 0) &&
        (resources.entryScanBlockCapacity > 0) &&
        splat_sort_resources_are_valid(resources.sort)
    );
}
