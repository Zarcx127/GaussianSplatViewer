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
