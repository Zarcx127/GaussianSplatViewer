#include "renderer/passes/SplatTileRenderPass.hpp"

#include "renderer/resources/splats/SplatFrame.hpp"

#include "renderer/FrameCommands.hpp"

void record_splat_tile_render_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet renderTargetDescriptorSet,
    vk::DescriptorSet splatFrameDescriptorSet,
    vk::Extent2D extent,
    uint32_t sortedBufferIndex
) {
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eCompute,
        pipeline
    );

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        0,
        1,
        &renderTargetDescriptorSet,
        0,
        nullptr
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

    SplatTileRenderPushConstant pushConstants = {};
    pushConstants.sortedBufferIndex = sortedBufferIndex;

    commandBuffer.pushConstants(
        pipelineLayout,
        vk::ShaderStageFlagBits::eCompute,
        sizeof(FramePushConstant),
        sizeof(SplatTileRenderPushConstant),
        &pushConstants
    );

    uint32_t groupCountX = (
        (extent.width / SPLAT_TILE_SIZE) +
        static_cast<uint32_t>(
            (extent.width % SPLAT_TILE_SIZE) != 0
        )
    );

    uint32_t groupCountY = (
        (extent.height / SPLAT_TILE_SIZE) +
        static_cast<uint32_t>(
            (extent.height % SPLAT_TILE_SIZE) != 0
        )
    );

    commandBuffer.dispatch(
        groupCountX,
        groupCountY,
        1
    );
}