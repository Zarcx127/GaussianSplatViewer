#include "factory/SplatFactory.hpp"

#include <vector>
#include <cstddef>

#include "renderer/resources/buffers/Buffer.hpp"

namespace
{
    std::vector<GpuSplat> pack_splats(const SplatCloud& cloud);
}

vk::VertexInputBindingDescription get_splat_vertex_binding_description()
{
    vk::VertexInputBindingDescription description = {};

    description.binding = 0;
    description.stride = sizeof(GpuSplat);
    description.inputRate = vk::VertexInputRate::eVertex;

    return description;
}

std::array<vk::VertexInputAttributeDescription, 2> get_splat_vertex_attribute_descriptions()
{
    std::array<vk::VertexInputAttributeDescription, 2> attributes;

    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = vk::Format::eR32G32B32Sfloat;
    attributes[0].offset = offsetof(GpuSplat, position);

    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = vk::Format::eR32G32B32A32Sfloat;
    attributes[1].offset = offsetof(GpuSplat, color);

    return attributes;
}

GraphicsPipelineConfig get_splat_point_pipeline_config()
{
    vk::VertexInputBindingDescription bindingDescription = 
        get_splat_vertex_binding_description();

    std::array<vk::VertexInputAttributeDescription, 2> atrributeDescription =
        get_splat_vertex_attribute_descriptions();

    GraphicsPipelineConfig pipelineConfig = {};

    pipelineConfig.vertexBindingDescriptions.push_back(bindingDescription);
    for(const vk::VertexInputAttributeDescription& attribute : atrributeDescription)
        pipelineConfig.vertexAttributeDescriptions.push_back(attribute);

    pipelineConfig.topology = vk::PrimitiveTopology::ePointList;

    pipelineConfig.cullMode = vk::CullModeFlagBits::eNone;
    pipelineConfig.frontFace = vk::FrontFace::eCounterClockwise;

    pipelineConfig.depthTest = true;
    pipelineConfig.depthWrite = true;
    pipelineConfig.depthCompareOp = vk::CompareOp::eLess;

    pipelineConfig.colorBlendAttachment.blendEnable = vk::False;
    pipelineConfig.colorBlendAttachment.colorWriteMask = (
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA 
    );

    return pipelineConfig;
}

SplatBuffer build_splat_buffer(
    const SplatCloud& cloud,
    VmaAllocator& allocator,
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
) {
    if(cloud.empty()) 
        return {};

    std::vector<GpuSplat> gpuSplats = pack_splats(cloud);
    
    const vk::DeviceSize ALLOC_SIZE = (sizeof(GpuSplat) * gpuSplats.size());
    AllocatedBuffer stagingBuffer = create_buffer(
        allocator,
        ALLOC_SIZE,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO,
        (
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT
        ),
        "Splat Staging Buffer"
    );

    if(!stagingBuffer.buffer)
        return {};

    if(!write_buffer(allocator, stagingBuffer, gpuSplats.data(), ALLOC_SIZE))
    {
        destroy_buffer(allocator, stagingBuffer);
        return {};
    }

    AllocatedBuffer gpuBuffer = create_buffer(
        allocator,
        ALLOC_SIZE,
        (
            vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eStorageBuffer |
            vk::BufferUsageFlagBits::eTransferDst
        ),
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        0,
        "Splat Buffer"
    );

    if(!gpuBuffer.buffer)
    {
        destroy_buffer(allocator, stagingBuffer);
        return {};
    }

    if(!copy_buffer(
        stagingBuffer.buffer, gpuBuffer.buffer, ALLOC_SIZE, 
        device, queue, commandPool
    )) {
        destroy_buffer(allocator, gpuBuffer);
        destroy_buffer(allocator, stagingBuffer);

        return {};
    }

    SplatBuffer splatBuffer = {};

    splatBuffer.buffer = gpuBuffer;
    splatBuffer.bufferOffset = 0;
    splatBuffer.splatCount = cloud.splat_count();
    splatBuffer.boundsMin = cloud.boundsMin;
    splatBuffer.boundsMax = cloud.boundsMax;

    destroy_buffer(allocator, stagingBuffer);

    deletionQueue.push_back(
        [splatBuffer] (VmaAllocator allocator) mutable->void {
            destroy_buffer(allocator, splatBuffer.buffer);
        }
    );

    return splatBuffer;
}

namespace
{
    std::vector<GpuSplat> pack_splats(const SplatCloud& cloud)
    {
        std::vector<GpuSplat> gpuSplats;
        gpuSplats.reserve(cloud.splat_count());

        for(const SplatVertex& splat : cloud.splats)
        {
            GpuSplat gpuSplat = {};

            gpuSplat.color = glm::vec4(splat.color, splat.opacity);
            gpuSplat.position = glm::vec4(splat.position, 1.0f);
            gpuSplat.logScale = glm::vec4(splat.logScale, 0.0f);
            gpuSplat.rotation = splat.rotation;

            gpuSplats.push_back(gpuSplat);
        }

        return gpuSplats;
    }
}
