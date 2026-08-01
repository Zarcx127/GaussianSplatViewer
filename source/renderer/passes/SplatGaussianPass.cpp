#include "renderer/passes/SplatGaussianPass.hpp"

void record_splat_gaussian_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
) {
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, 
        pipeline
    );

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        pipelineLayout,
        2,
        1,
        &splatFrameDescriptorSet,
        0,
        nullptr
    );

    commandBuffer.drawIndirect(
        resources.drawCommand.buffer,
        0,
        1,
        sizeof(GpuSplatDrawCommand)
    );
}