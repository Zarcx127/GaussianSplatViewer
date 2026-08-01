#include "renderer/passes/SplatSortPass.hpp"

#include <array>
#include <vector>
#include <utility>

#include "renderer/core/Synchronization.hpp"

#include "renderer/FrameCommands.hpp"

namespace
{
    struct SplatSortScanLevel
    {
        uint32_t offset { 0 };
        uint32_t valueCount { 0 };
        uint32_t blockCount { 0 };
    };

    void record_splat_sort_digit(
        vk::CommandBuffer commandBuffer,
        const SplatSortPipelines& pipelines,
        vk::PipelineLayout pipelineLayout,
        const SplatFrameResources& resources,
        const std::vector<SplatSortScanLevel>& scanLevels,
        const SplatSortPushConstant& pushConstants
    );

    uint32_t divide_round_up(uint32_t value, uint32_t divisor);

    uint32_t calculate_required_bit_count(uint32_t maximumValue);

    std::vector<SplatSortScanLevel> build_splat_sort_scan_levels(
        uint32_t firstLevelValueCount
    );
}

uint32_t record_splat_sort_pass(
    vk::CommandBuffer commandBuffer,
    const SplatSortPipelines& pipelines,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
) {
    std::vector<SplatSortScanLevel> scanLevels = 
        build_splat_sort_scan_levels(resources.sort.histogramBlockCapacity);

    if(scanLevels.empty())
        return 0;

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        2,
        1,
        &splatFrameDescriptorSet,
        0,
        nullptr
    );

    uint32_t inputBufferIndex = 0;
    uint32_t outputBufferIndex = 1;

    SplatSortPushConstant pushConstants = {};
    pushConstants.keyComponent = static_cast<uint32_t>(SplatSortKeyComponent::Depth);

    for(
        uint32_t digitShift = 0; 
        digitShift < SPLAT_SORT_KEY_BIT_COUNT; 
        digitShift += SPLAT_SORT_RADIX_BITS
    ) {
        pushConstants.digitShift = digitShift;
        pushConstants.inputBufferIndex = inputBufferIndex;
        pushConstants.outputBufferIndex = outputBufferIndex;

        record_splat_sort_digit(
            commandBuffer, pipelines, pipelineLayout,
            resources, scanLevels, pushConstants
        );

        std::swap(inputBufferIndex, outputBufferIndex);
    }

    uint32_t tileBitCount = calculate_required_bit_count(
        resources.tileCapacity - 1
    );

    pushConstants.keyComponent =
        static_cast<uint32_t>(SplatSortKeyComponent::Tile);

    for(
        uint32_t digitShift = 0; 
        digitShift < tileBitCount; 
        digitShift += SPLAT_SORT_RADIX_BITS
    ) {
        pushConstants.digitShift = digitShift;
        pushConstants.inputBufferIndex = inputBufferIndex;
        pushConstants.outputBufferIndex = outputBufferIndex;

        record_splat_sort_digit(
            commandBuffer, pipelines, pipelineLayout,
            resources, scanLevels, pushConstants
        );

        std::swap(inputBufferIndex, outputBufferIndex);
    }

    return inputBufferIndex;
}

namespace
{
    void record_splat_sort_digit(
        vk::CommandBuffer commandBuffer,
        const SplatSortPipelines& pipelines,
        vk::PipelineLayout pipelineLayout,
        const SplatFrameResources& resources,
        const std::vector<SplatSortScanLevel>& scanLevels,
        const SplatSortPushConstant& pushConstants
    ) {
        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eCompute,
            pipelines.histogram
        );

        commandBuffer.pushConstants(
            pipelineLayout,
            vk::ShaderStageFlagBits::eCompute,
            sizeof(FramePushConstant),
            sizeof(SplatSortPushConstant),
            &pushConstants
        );

        commandBuffer.dispatchIndirect(
            resources.sort.dispatchCommands.buffer,
            SPLAT_SORT_ENTRY_DISPATCH_OFFSET
        );

        vk::BufferMemoryBarrier histogramBarrier = make_buffer_memory_barrier(
            resources.sort.radixHistograms.buffer,
            resources.sort.radixHistograms.size,
            vk::AccessFlagBits::eShaderWrite,
            (
                vk::AccessFlagBits::eShaderRead |
                vk::AccessFlagBits::eShaderWrite
            )
        );

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags(),
            nullptr,
            histogramBarrier,
            nullptr
        );

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eCompute,
            pipelines.histogramLocalScan
        );

        commandBuffer.dispatch(
            resources.sort.histogramBlockCapacity,
            SPLAT_SORT_RADIX_BUCKET_COUNT,
            1
        );

        std::array<vk::BufferMemoryBarrier, 2> localScanBarriers = {
            make_buffer_memory_barrier(
                resources.sort.radixHistograms.buffer,
                resources.sort.radixHistograms.size,
                vk::AccessFlagBits::eShaderWrite,
                (
                    vk::AccessFlagBits::eShaderRead |
                    vk::AccessFlagBits::eShaderWrite
                )
            ),
            make_buffer_memory_barrier(
                resources.sort.radixScanBlockSums.buffer,
                resources.sort.radixScanBlockSums.size,
                vk::AccessFlagBits::eShaderWrite,
                (
                    vk::AccessFlagBits::eShaderRead |
                    vk::AccessFlagBits::eShaderWrite
                )
            )
        };

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags(),
            nullptr,
            localScanBarriers,
            nullptr
        );

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eCompute,
            pipelines.histogramBlockScan
        );

        for(size_t levelIndex = 0; levelIndex < scanLevels.size(); levelIndex++)
        {
            const SplatSortScanLevel& level = scanLevels[levelIndex];
            bool isFinalLevel = ((levelIndex + 1) == scanLevels.size());

            SplatSortScanPushConstant scanPushConstants = {};

            scanPushConstants.valueCount = level.valueCount;
            scanPushConstants.inputOffset = level.offset;
            scanPushConstants.outputOffset = (
                isFinalLevel
                    ? 0
                    : scanLevels[levelIndex + 1].offset
            );

            scanPushConstants.writeBucketTotal = 
                static_cast<uint32_t>(isFinalLevel);

            commandBuffer.pushConstants(
                pipelineLayout,
                vk::ShaderStageFlagBits::eCompute,
                sizeof(FramePushConstant),
                sizeof(SplatSortScanPushConstant),
                &scanPushConstants
            );

            commandBuffer.dispatch(
                level.blockCount,
                SPLAT_SORT_RADIX_BUCKET_COUNT,
                1
            );

            vk::BufferMemoryBarrier blockScanBarrier = make_buffer_memory_barrier(
                resources.sort.radixScanBlockSums.buffer,
                resources.sort.radixScanBlockSums.size,
                vk::AccessFlagBits::eShaderWrite,
                (
                    vk::AccessFlagBits::eShaderRead |
                    vk::AccessFlagBits::eShaderWrite
                )
            );

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlags(),
                nullptr,
                blockScanBarrier,
                nullptr
            );
        }

        vk::BufferMemoryBarrier bucketTotalBarrier = make_buffer_memory_barrier(
            resources.sort.radixBucketOffsets.buffer,
            resources.sort.radixBucketOffsets.size,
            vk::AccessFlagBits::eShaderWrite,
            (
                vk::AccessFlagBits::eShaderRead |
                vk::AccessFlagBits::eShaderWrite
            )
        );

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags(),
            nullptr,
            bucketTotalBarrier,
            nullptr
        );

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eCompute,
            pipelines.histogramAddBlockOffsets
        );

        for(size_t levelIndex = scanLevels.size(); levelIndex > 1; levelIndex--)
        {
            const SplatSortScanLevel& targetLevel = scanLevels[levelIndex - 2];
            const SplatSortScanLevel& blockLevel = scanLevels[levelIndex - 1];

            SplatSortAddOffsetsPushConstant offsetPushConstants = {};

            offsetPushConstants.valueCount = targetLevel.valueCount;
            offsetPushConstants.targetOffset = targetLevel.offset;
            offsetPushConstants.blockOffset = blockLevel.offset;
            offsetPushConstants.targetBuffer = 
                static_cast<uint32_t>(SplatSortScanTarget::BlockSums);

            commandBuffer.pushConstants(
                pipelineLayout,
                vk::ShaderStageFlagBits::eCompute,
                sizeof(FramePushConstant),
                sizeof(SplatSortAddOffsetsPushConstant),
                &offsetPushConstants
            );

            commandBuffer.dispatch(
                targetLevel.blockCount,
                SPLAT_SORT_RADIX_BUCKET_COUNT,
                1
            );

            vk::BufferMemoryBarrier blockOffsetBarrier = make_buffer_memory_barrier(
                resources.sort.radixScanBlockSums.buffer,
                resources.sort.radixScanBlockSums.size,
                vk::AccessFlagBits::eShaderWrite,
                (
                    vk::AccessFlagBits::eShaderRead |
                    vk::AccessFlagBits::eShaderWrite
                )
            );

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlags(),
                nullptr,
                blockOffsetBarrier,
                nullptr
            );
        }

        SplatSortAddOffsetsPushConstant histogramOffsetPushConstants = {};

        histogramOffsetPushConstants.valueCount = resources.sort.workgroupCapacity;
        histogramOffsetPushConstants.targetOffset = 0;
        histogramOffsetPushConstants.blockOffset = scanLevels.front().offset;
        histogramOffsetPushConstants.targetBuffer = 
            static_cast<uint32_t>(SplatSortScanTarget::Histograms);

        commandBuffer.pushConstants(
            pipelineLayout,
            vk::ShaderStageFlagBits::eCompute,
            sizeof(FramePushConstant),
            sizeof(SplatSortAddOffsetsPushConstant),
            &histogramOffsetPushConstants
        );

        commandBuffer.dispatch(
            resources.sort.histogramBlockCapacity,
            SPLAT_SORT_RADIX_BUCKET_COUNT,
            1
        );

        vk::BufferMemoryBarrier histogramOffsetBarrier = make_buffer_memory_barrier(
            resources.sort.radixHistograms.buffer,
            resources.sort.radixHistograms.size,
            vk::AccessFlagBits::eShaderWrite,
            (
                vk::AccessFlagBits::eShaderRead |
                vk::AccessFlagBits::eShaderWrite
            )
        );

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags(),
            nullptr,
            histogramOffsetBarrier,
            nullptr
        );

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eCompute,
            pipelines.bucketOffsetScan
        );

        commandBuffer.dispatch(
            1,
            1,
            1
        );

        vk::BufferMemoryBarrier bucketOffsetBarrier = make_buffer_memory_barrier(
            resources.sort.radixBucketOffsets.buffer,
            resources.sort.radixBucketOffsets.size,
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead
        );

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags(),
            nullptr,
            bucketOffsetBarrier,
            nullptr
        );

        commandBuffer.bindPipeline(
            vk::PipelineBindPoint::eCompute,
            pipelines.scatter
        );

        commandBuffer.pushConstants(
            pipelineLayout,
            vk::ShaderStageFlagBits::eCompute,
            sizeof(FramePushConstant),
            sizeof(SplatSortPushConstant),
            &pushConstants
        );

        commandBuffer.dispatchIndirect(
            resources.sort.dispatchCommands.buffer,
            SPLAT_SORT_ENTRY_DISPATCH_OFFSET
        );

        uint32_t outputBufferIndex = pushConstants.outputBufferIndex;
        std::array<vk::BufferMemoryBarrier, 2> scatterBarriers = {
            make_buffer_memory_barrier(
                resources.entryKeys[outputBufferIndex].buffer,
                resources.entryKeys[outputBufferIndex].size,
                vk::AccessFlagBits::eShaderWrite,
                vk::AccessFlagBits::eShaderRead
            ),
            make_buffer_memory_barrier(
                resources.entrySplatIndices[outputBufferIndex].buffer,
                resources.entrySplatIndices[outputBufferIndex].size,
                vk::AccessFlagBits::eShaderWrite,
                vk::AccessFlagBits::eShaderRead
            )
        };

        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags(),
            nullptr,
            scatterBarriers,
            nullptr
        );
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

    uint32_t calculate_required_bit_count(uint32_t maximumValue)
    {
        uint32_t bitCount = 0;
        while(maximumValue > 0)
        {
            bitCount++;
            maximumValue >>= 1;
        }

        return bitCount;
    }

    std::vector<SplatSortScanLevel> build_splat_sort_scan_levels(
        uint32_t firstLevelValueCount
    ) {
        std::vector<SplatSortScanLevel> levels;

        uint32_t levelOffset = 0;
        uint32_t levelValueCount = firstLevelValueCount;

        while(levelValueCount > 0)
        {
            uint32_t blockCount = divide_round_up(
                levelValueCount, SPLAT_SORT_SCAN_LOCAL_SIZE
            );

            levels.push_back({
                levelOffset,
                levelValueCount,
                blockCount
            });

            if(blockCount == 1)
                break;

            levelOffset += levelValueCount;
            levelValueCount = blockCount;
        }

        return levels;
    }
}
