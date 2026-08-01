#include "renderer/passes/SplatPointPass.hpp"

void record_splat_point_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    const SplatBuffer& splatBuffer
) {
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics, 
        pipeline
    );

    commandBuffer.bindVertexBuffers(
        0, 1, &(splatBuffer.buffer.buffer), &(splatBuffer.bufferOffset)
    );

    commandBuffer.draw(splatBuffer.splatCount, 1, 0, 0);
}