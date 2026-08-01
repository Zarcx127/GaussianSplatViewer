#include "renderer/passes/BackgroundPass.hpp"

void record_background_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet descriptorSet,
    vk::Extent2D extent
) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );

    uint32_t pixelCount = extent.width * extent.height;
    uint32_t groupCount = (pixelCount + 63) / 64;

    commandBuffer.dispatch(groupCount, 1, 1);
}