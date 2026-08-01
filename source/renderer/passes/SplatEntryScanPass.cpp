#include "renderer/passes/SplatEntryScanPass.hpp"

#include <array>

#include "renderer/core/Synchronization.hpp"

void record_splat_entry_scan_pass(
    vk::CommandBuffer commandBuffer,
    const SplatEntryScanPipelines& pipelines,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
) {
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        2,
        1,
        &splatFrameDescriptorSet,
        0,
        nullptr
    );

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipelines.localScan
    );


    commandBuffer.dispatch(
        resources.entryScanBlockCapacity,
        1,
        1
    );

    std::array<vk::BufferMemoryBarrier, 2> outputBarriers = {
        make_buffer_memory_barrier(
            resources.entryRanges.buffer,
            resources.entryRanges.size,
            vk::AccessFlagBits::eShaderWrite,
            (
                vk::AccessFlagBits::eShaderRead |
                vk::AccessFlagBits::eShaderWrite
            )
        ),
        make_buffer_memory_barrier(
            resources.entryScanBlockSums.buffer,
            resources.entryScanBlockSums.size,
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
        outputBarriers,
        nullptr
    );

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipelines.blockScan
    );

    commandBuffer.dispatch(1, 1, 1);

    vk::BufferMemoryBarrier blockScanBarrier = make_buffer_memory_barrier(
        resources.entryScanBlockSums.buffer,
        resources.entryScanBlockSums.size,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eShaderRead
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        blockScanBarrier,
        nullptr
    );

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipelines.addBlockOffsets
    );

    commandBuffer.dispatch(
        resources.entryScanBlockCapacity,
        1,
        1
    );

    vk::BufferMemoryBarrier offsetBarrier = make_buffer_memory_barrier(
        resources.entryRanges.buffer,
        resources.entryRanges.size,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eShaderRead
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        offsetBarrier,
        nullptr
    );

    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipelines.finalize
    );

    commandBuffer.dispatch(1, 1, 1);

    vk::BufferMemoryBarrier finalizeBarrier = make_buffer_memory_barrier(
        resources.counters.buffer,
        resources.counters.size,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eShaderRead
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        finalizeBarrier,
        nullptr
    );
}
