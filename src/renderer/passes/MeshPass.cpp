#include "renderer/passes/MeshPass.hpp"

void record_mesh_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    const Mesh& mesh
) {
    commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        pipeline
    );

    commandBuffer.bindVertexBuffers(
        0, 1, &(mesh.vertexBuffer.buffer), &(mesh.vertexBufferOffset)
    );
    
    commandBuffer.draw(mesh.vertexCount, 1, 0, 0);
}