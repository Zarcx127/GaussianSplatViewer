#include "renderer/passes/SplatTileRangePass.hpp"

#include "renderer/core/Synchronization.hpp"

#include "renderer/FrameCommands.hpp"

namespace
{
    constexpr uint32_t SPLAT_TILE_RANGE_LOCAL_SIZE = 256;
    constexpr uint32_t SPLAT_TILE_RANGE_DISPATCH_ROW_CAPACITY = 65535;

    uint32_t divide_round_up(uint32_t value, uint32_t divisor);
};

void record_splat_tile_range_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources,
    uint32_t sortedBufferIndex
) {
    commandBuffer.fillBuffer(
        resources.tileRanges.buffer,
        0,
        resources.tileRanges.size,
        0
    );

    vk::BufferMemoryBarrier clearBarrier = make_buffer_memory_barrier(
        resources.tileRanges.buffer,
        resources.tileRanges.size,
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eShaderWrite
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        clearBarrier,
        nullptr
    );

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipeline
    );

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        2,
        1,
        &splatFrameDescriptorSet,
        0,
        nullptr
    );

    SplatTileRangePushConstant pushConstants = {};
    pushConstants.sortedBufferIndex = sortedBufferIndex;

    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eCompute,
        sizeof(FramePushConstant),
        sizeof(SplatTileRangePushConstant),
        &pushConstants
    );

    uint32_t workgroupCapacity = divide_round_up(
        resources.entryCapacity,
        SPLAT_TILE_RANGE_LOCAL_SIZE
    );

    uint32_t groupCountY = divide_round_up(
        workgroupCapacity,
        SPLAT_TILE_RANGE_DISPATCH_ROW_CAPACITY
    );

    uint32_t groupCountX = divide_round_up(
        workgroupCapacity,
        groupCountY
    );

    commandBuffer.dispatch(
        groupCountX,
        groupCountY,
        1
    );

    vk::BufferMemoryBarrier rangeBarrier = make_buffer_memory_barrier(
        resources.tileRanges.buffer,
        resources.tileRanges.size,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eShaderRead
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        rangeBarrier,
        nullptr
    );
}

namespace
{
    uint32_t divide_round_up(uint32_t value, uint32_t divisor)
    {
        if(divisor == 0)
            return 0;

        return (
            (value / divisor) +
            static_cast<uint32_t>((value % divisor) != 0)
        );
    }
}