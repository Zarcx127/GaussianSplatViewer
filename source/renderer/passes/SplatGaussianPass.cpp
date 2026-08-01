#include "renderer/passes/SplatGaussianPass.hpp"

void record_splat_gaussian_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet sphericalHarmonicDescriptorSet,
    const SplatBuffer& splatBuffer
) {
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, 
        pipeline
    );

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        1,
        1,
        &sphericalHarmonicDescriptorSet,
        0,
        nullptr
    );

    commandBuffer.bindVertexBuffers(
        0, 1, &(splatBuffer.buffer.buffer), &(splatBuffer.bufferOffset)
    );

    commandBuffer.draw(6, splatBuffer.splatCount, 0, 0);
}