#include "factory/SplatFrameFactory.hpp"

#include <limits>
#include <algorithm>

#include <vulkan/vulkan.hpp>

namespace
{
    constexpr vk::DeviceSize SPLAT_ENTRY_MEMORY_BUDGET = (
        128ULL * 1024ULL * 1024ULL
    );

    constexpr vk::DeviceSize SPLAT_ENTRY_STORAGE_SIZE = (
        (2ULL * sizeof(GpuSplatEntryKey)) +
        (2ULL * sizeof(uint32_t))
    );

    AllocatedBuffer create_processing_buffer(
        VmaAllocator allocator,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        const char* name
    );

    void destroy_splat_frame_resources(
        VmaAllocator allocator,
        SplatFrameResources& resources
    );

    uint32_t divide_round_up(uint32_t value, uint32_t divisor);
    uint32_t calculate_scan_block_capacity(uint32_t valueCapacity);
}

SplatFrameResources build_splat_frame_resources(
    VmaAllocator allocator,
    uint32_t splatCapacity,
    uint32_t entryCapacity,
    uint32_t tileCapacity,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
) {
    if(
        !allocator || 
        (splatCapacity == 0) || 
        (entryCapacity == 0) ||
        (tileCapacity == 0)
    ) {
        return {};
    }

    SplatFrameResources resources = {};

    resources.splatCapacity = splatCapacity;
    resources.entryCapacity = entryCapacity;
    resources.tileCapacity = tileCapacity;
    resources.entryScanBlockCapacity = (
        (splatCapacity  + SPLAT_ENTRY_SCAN_LOCAL_SIZE - 1) /
        SPLAT_ENTRY_SCAN_LOCAL_SIZE
    );

    resources.sort.workgroupCapacity = divide_round_up(
        entryCapacity, SPLAT_SORT_LOCAL_SIZE
    );

    resources.sort.histogramBlockCapacity = divide_round_up(
        resources.sort.workgroupCapacity, SPLAT_SORT_SCAN_LOCAL_SIZE
    );

    resources.sort.scanBlockCapacityPerBucket = 
        calculate_scan_block_capacity(resources.sort.workgroupCapacity);

    uint64_t histogramCapacity = (
        static_cast<uint64_t>(resources.sort.workgroupCapacity) *
        SPLAT_SORT_RADIX_BUCKET_COUNT
    );

    if(histogramCapacity > std::numeric_limits<uint32_t>::max())
        return {};
    
    resources.sort.histogramCapacity = 
        static_cast<uint32_t>(histogramCapacity);

    if(
        (resources.sort.workgroupCapacity == 0) ||
        (resources.sort.histogramCapacity == 0) ||
        (resources.sort.histogramBlockCapacity == 0) ||
        (resources.sort.scanBlockCapacityPerBucket == 0)
    ) {
        return {};
    }

    const vk::DeviceSize ENTRY_KEY_BUFFER_SIZE = (
        (sizeof(GpuSplatEntryKey) * static_cast<vk::DeviceSize>(entryCapacity))
    );

    const vk::DeviceSize INDEX_BUFFER_SIZE = (
        (sizeof(uint32_t) * static_cast<vk::DeviceSize>(entryCapacity))
    );

// FLAG //
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

    resources.entryRanges = create_processing_buffer(
        allocator,
        (sizeof(GpuSplatEntryRange) * static_cast<vk::DeviceSize>(splatCapacity)),
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Entry Range Buffer"
    );

    resources.entryKeys[0] = create_processing_buffer(
        allocator,
        ENTRY_KEY_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Entry Key Buffer A"
    );

    resources.entryKeys[1] = create_processing_buffer(
        allocator,
        ENTRY_KEY_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Entry Key Buffer B"
    );

    resources.entrySplatIndices[0] = create_processing_buffer(
        allocator,
        INDEX_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Entry Splat Index Buffer A"
    );

    resources.entrySplatIndices[1] = create_processing_buffer(
        allocator,
        INDEX_BUFFER_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Entry Splat Index Buffer B"
    );

    resources.counters = create_processing_buffer(
        allocator,
        sizeof(GpuSplatCounters),
        (
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eTransferSrc |
            vk::BufferUsageFlagBits::eTransferDst 
        ),
        "Splat Counter Buffer"
    );

    resources.counterReadback = create_buffer(
        allocator,
        sizeof(GpuSplatCounters),
        vk::BufferUsageFlagBits::eTransferDst,
        VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        (
            VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
            VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT
        ),
        "Splat Counter Readback Buffer"
    );

    resources.drawCommand = create_processing_buffer(
        allocator,
        sizeof(vk::DrawIndirectCommand),
        (
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eIndirectBuffer |
            vk::BufferUsageFlagBits::eTransferDst
        ),
        "Splat Draw Command Buffer"
    );

    resources.tileRanges = create_processing_buffer(
        allocator,
        (
            sizeof(GpuSplatTileRange) *
            static_cast<vk::DeviceSize>(tileCapacity)
        ),
        (
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eTransferDst
        ),
        "Splat Tile Bin Buffer"
    );

    resources.entryScanBlockSums = create_processing_buffer(
        allocator,
        (
            sizeof(uint32_t) *
            static_cast<vk::DeviceSize>(resources.entryScanBlockCapacity)
        ),
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Entry Scan Block Sum Buffer"
    );

    resources.sort.radixHistograms = create_processing_buffer(
        allocator,
        (
            sizeof(uint32_t) *
            static_cast<vk::DeviceSize>(resources.sort.histogramCapacity)
        ),
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Sort Radix Histogram Buffer"
    );

    resources.sort.radixScanBlockSums = create_processing_buffer(
        allocator,
        (
            sizeof(uint32_t) * 
            static_cast<vk::DeviceSize>(resources.sort.scanBlockCapacityPerBucket) *
            static_cast<vk::DeviceSize>(SPLAT_SORT_RADIX_BUCKET_COUNT)
        ),
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Sort Radix Scan Block Sum Buffer"
    );

    resources.sort.radixBucketOffsets = create_processing_buffer(
        allocator,
        (sizeof(uint32_t) * SPLAT_SORT_RADIX_BUCKET_COUNT),
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Splat Sort Radix Bucket Offset Buffer"
    );
/////

    if(!splat_frame_resources_are_valid(resources))
    {
        destroy_splat_frame_resources(allocator, resources);
        return {};
    }

    GpuSplatCounters initialCounters = {};

    if(!write_buffer(
        allocator,
        resources.counterReadback,
        &initialCounters,
        sizeof(initialCounters)
    )) {
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

uint32_t calculate_splat_entry_capacity(
    vk::PhysicalDevice physicalDevice,
    uint32_t splatCapacity,
    uint32_t framesInFlight
) {
    if(
        !physicalDevice ||
        (splatCapacity == 0) ||
        (framesInFlight == 0)
    ) {
        return 0;
    }

    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

    vk::DeviceSize maximumBufferRange = properties.limits.maxStorageBufferRange;
    vk::DeviceSize maximumEntryCapacity = std::min(
        (maximumBufferRange / sizeof(GpuSplatEntryKey)),
        (maximumBufferRange / sizeof(uint32_t))
    );

    vk::DeviceSize frameBudget = (
        SPLAT_ENTRY_MEMORY_BUDGET / 
        static_cast<vk::DeviceSize>(framesInFlight)
    );

    vk::DeviceSize budgetCapacity = (
        frameBudget / SPLAT_ENTRY_STORAGE_SIZE
    );

    vk::DeviceSize entryCapacity = std::max(
        static_cast<vk::DeviceSize>(splatCapacity),
        budgetCapacity
    );

    entryCapacity = std::min(
        entryCapacity, 
        maximumEntryCapacity
    );

    entryCapacity = std::min(
        entryCapacity,
        static_cast<vk::DeviceSize>(std::numeric_limits<uint32_t>::max())
    );

    if(entryCapacity < splatCapacity)
        return 0;

    return static_cast<uint32_t>(entryCapacity);
}

uint32_t calculate_grown_splat_entry_capacity(
    vk::PhysicalDevice physicalDevice,
    uint32_t currentCapacity,
    uint32_t requiredCapacity
) {
    if(
        !physicalDevice ||
        (currentCapacity == 0) ||
        (requiredCapacity == 0)
    ) {
        return 0;
    }

    if(requiredCapacity <= currentCapacity)
        return currentCapacity;

    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
    vk::DeviceSize maximumBufferRange = properties.limits.maxStorageBufferRange;
    uint64_t maximumEntryCapacity = std::min(
        static_cast<uint64_t>(maximumBufferRange / sizeof(GpuSplatEntryKey)),
        static_cast<uint64_t>(maximumBufferRange / sizeof(uint32_t))
    );

    maximumEntryCapacity = std::min(
        maximumEntryCapacity,
        static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())
    );

    uint64_t currentGrowth = (
        static_cast<uint64_t>(currentCapacity) +
        std::max<uint64_t>((currentCapacity / 2), 1)
    );

    uint64_t requiredGrowth = (
        static_cast<uint64_t>(requiredCapacity) +
        std::max<uint64_t>((requiredCapacity / 4), 1)
    );

    uint64_t grownCapacity = std::max(currentGrowth, requiredGrowth);
    grownCapacity = std::min(grownCapacity, maximumEntryCapacity);

    if(grownCapacity < requiredCapacity)
        return 0;

    return static_cast<uint32_t>(grownCapacity);
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

    void destroy_splat_frame_resources(
        VmaAllocator allocator,
        SplatFrameResources& resources
    ) {
        destroy_buffer(allocator, resources.projectedSplats);
        destroy_buffer(allocator, resources.visibleSplatIndices);
        destroy_buffer(allocator, resources.entryRanges);

        for(AllocatedBuffer& buffer : resources.entryKeys)
            destroy_buffer(allocator, buffer);
        
        for(AllocatedBuffer& buffer : resources.entrySplatIndices)
            destroy_buffer(allocator, buffer);
    
        destroy_buffer(allocator, resources.counters);
        destroy_buffer(allocator, resources.counterReadback);
        destroy_buffer(allocator, resources.drawCommand);
        destroy_buffer(allocator, resources.tileRanges);
        destroy_buffer(allocator, resources.entryScanBlockSums);
        destroy_buffer(allocator, resources.sort.radixHistograms);
        destroy_buffer(allocator, resources.sort.radixScanBlockSums);
        destroy_buffer(allocator, resources.sort.radixBucketOffsets);

        resources = {};
    }

    uint32_t divide_round_up(uint32_t value, uint32_t divisor)
    {
        if(divisor == 0)
            return 0;

        return (
            (value / divisor) +
            static_cast<uint32_t>((value % divisor) != 0)
        );
    }

    uint32_t calculate_scan_block_capacity(uint32_t valueCapacity)
    {
        uint32_t levelCapacity = divide_round_up(
            valueCapacity, SPLAT_SORT_SCAN_LOCAL_SIZE
        );

        uint64_t totalCapacity = 0;

        while(levelCapacity > 0)
        {
            totalCapacity += levelCapacity;
            if(totalCapacity > std::numeric_limits<uint32_t>::max())
                return 0;

            if(levelCapacity <= SPLAT_SORT_SCAN_LOCAL_SIZE)
                break;

            levelCapacity = divide_round_up(
                levelCapacity, SPLAT_SORT_SCAN_LOCAL_SIZE
            );
        }

        return static_cast<uint32_t>(totalCapacity);
    }
}
