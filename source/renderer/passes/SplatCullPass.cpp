#include "renderer/passes/SplatCullPass.hpp"

#include <array>

namespace 
{
    vk::BufferMemoryBarrier make_buffer_barrier(
        vk::Buffer buffer,
        vk::DeviceSize size,
        vk::AccessFlags sourceAccess,
        vk::AccessFlags destinationAccess
    );
}

void record_splat_cull_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet sphericalHarmonicDescriptorSet,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
) {
    commandBuffer.fillBuffer(
        resources.counters.buffer, 0, resources.counters.size, 0
    );

    GpuSplatDrawCommand drawCommand = {};
    drawCommand.vertexCount = 6;

    commandBuffer.updateBuffer(
        resources.drawCommand.buffer,
        0,
        sizeof(GpuSplatDrawCommand),
        &drawCommand
    );

    std::array<vk::BufferMemoryBarrier, 2> resetBarriers = {
        make_buffer_barrier(
            resources.counters.buffer,
            resources.counters.size,
            vk::AccessFlagBits::eTransferWrite,
            (
                vk::AccessFlagBits::eShaderRead |
                vk::AccessFlagBits::eShaderWrite
            )
        ),
        make_buffer_barrier(
            resources.drawCommand.buffer,
            resources.counters.size,
            vk::AccessFlagBits::eTransferWrite,
            (
                vk::AccessFlagBits::eShaderRead |
                vk::AccessFlagBits::eShaderWrite
            )
        ),
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlagBits(),
        nullptr,
        resetBarriers,
        nullptr
    );

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

    std::array<vk::DescriptorSet, 2> descriptorSets = {
        sphericalHarmonicDescriptorSet,
        splatFrameDescriptorSet
    };

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        1,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        0,
        nullptr
    );

    uint32_t groupCount = ((resources.splatCapacity + 255) / 256);
    commandBuffer.dispatch(groupCount, 1, 1);

    std::array<vk::BufferMemoryBarrier, 3> renderBarriers = {
        make_buffer_barrier(
            resources.projectedSplats.buffer,
            resources.projectedSplats.size,
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead
        ),
        make_buffer_barrier(
            resources.entrySplatIndices[0].buffer,
            resources.entrySplatIndices[1].size,
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eShaderRead
        ),
        make_buffer_barrier(
            resources.drawCommand.buffer,
            resources.drawCommand.size,
            vk::AccessFlagBits::eShaderWrite,
            vk::AccessFlagBits::eIndirectCommandRead
        )
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        (
            vk::PipelineStageFlagBits::eVertexShader |
            vk::PipelineStageFlagBits::eDrawIndirect
        ),
        vk::DependencyFlags(),
        nullptr,
        renderBarriers,
        nullptr
    );
}

namespace 
{
    vk::BufferMemoryBarrier make_buffer_barrier(
        vk::Buffer buffer,
        vk::DeviceSize size,
        vk::AccessFlags sourceAccess,
        vk::AccessFlags destinationAccess
    ) {
        vk::BufferMemoryBarrier barrier = {};

        barrier.srcAccessMask = sourceAccess;
        barrier.dstAccessMask = destinationAccess;
        barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
        barrier.buffer = buffer;
        barrier.offset = 0;
        barrier.size = size;

        return barrier;
    }
}
