#include "factory/SplatFactory.hpp"

#include <vector>
#include <cstddef>
#include <algorithm>

#include "renderer/resources/buffers/Buffer.hpp"

namespace
{
    vk::VertexInputBindingDescription get_splat_vertex_binding_description();
    std::array<vk::VertexInputAttributeDescription, 4> get_splat_vertex_attribute_descriptions();

    std::vector<GpuSplat> pack_splats(const SplatCloud& cloud);

    AllocatedBuffer upload_splat_data(
        const void* data,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        const char* stagingName,
        const char* bufferName,
        VmaAllocator& allocator,
        vk::Device& device,
        vk::CommandPool commandPool,
        vk::Queue& queue
    );

    std::vector<glm::vec4> pack_spherical_harmonics(const SplatCloud& cloud);
}

GraphicsPipelineConfig get_splat_gaussian_pipeline_config()
{
    vk::VertexInputBindingDescription bindingDescription = 
        get_splat_vertex_binding_description();

    std::array<vk::VertexInputAttributeDescription, 4> atrributeDescription =
        get_splat_vertex_attribute_descriptions();

    GraphicsPipelineConfig pipelineConfig = {};

    pipelineConfig.vertexBindingDescriptions.push_back(bindingDescription);
    for(const vk::VertexInputAttributeDescription& attribute : atrributeDescription)
        pipelineConfig.vertexAttributeDescriptions.push_back(attribute);

    pipelineConfig.topology = vk::PrimitiveTopology::eTriangleList;

    pipelineConfig.cullMode = vk::CullModeFlagBits::eNone;
    pipelineConfig.frontFace = vk::FrontFace::eCounterClockwise;

    pipelineConfig.depthTest = true;
    pipelineConfig.depthWrite = false;
    pipelineConfig.depthCompareOp = vk::CompareOp::eLess;

    pipelineConfig.colorBlendAttachment.blendEnable = vk::True;
    pipelineConfig.colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
    pipelineConfig.colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    pipelineConfig.colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
    pipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
    pipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
    pipelineConfig.colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
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
    AllocatedBuffer gpuBuffer = upload_splat_data(
        gpuSplats.data(),
        ALLOC_SIZE,
        (
            vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eStorageBuffer
        ),
        "Splat Staging Buffer",        
        "Splat Buffer",
        allocator, 
        device, 
        commandPool, 
        queue
    );

    if(!gpuBuffer.buffer)
        return {};

    SplatBuffer splatBuffer = {};

    splatBuffer.buffer = gpuBuffer;
    splatBuffer.bufferOffset = 0;
    splatBuffer.splatCount = cloud.splat_count();
    splatBuffer.boundsMin = cloud.boundsMin;
    splatBuffer.boundsMax = cloud.boundsMax;

    deletionQueue.push_back(
        [splatBuffer] (VmaAllocator allocator) mutable->void {
            destroy_buffer(allocator, splatBuffer.buffer);
        }
    );

    return splatBuffer;
}

SphericalHarmonicBuffer build_spherical_harmonic_buffer(
    const SplatCloud& cloud,
    VmaAllocator& allocator,
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
) {
    std::vector<glm::vec4> sphericalHarmonics = pack_spherical_harmonics(cloud);

    const vk::DeviceSize ALLOC_SIZE = (
        sizeof(glm::vec4) * sphericalHarmonics.size()
    );

    AllocatedBuffer gpuBuffer = upload_splat_data(
        sphericalHarmonics.data(),
        ALLOC_SIZE,
        vk::BufferUsageFlagBits::eStorageBuffer,
        "Spherical Harmonic Staging Buffer",
        "Spherical Harmonic Buffer",
        allocator,
        device,
        commandPool,
        queue
    );

    if(!gpuBuffer.buffer)
        return {};

    SphericalHarmonicBuffer buffer = {};

    buffer.buffer = gpuBuffer;
    buffer.degree = cloud.info.sphericalHarmonicDegree;
    buffer.coefficientCount = cloud.spherical_harmonic_coefficient_count();

    deletionQueue.push_back(
        [buffer] (VmaAllocator allocator) mutable->void {
            destroy_buffer(allocator, buffer.buffer);
        }
    );

    return buffer;
}

namespace
{
    vk::VertexInputBindingDescription get_splat_vertex_binding_description()
    {
        vk::VertexInputBindingDescription description = {};

        description.binding = 0;
        description.stride = sizeof(GpuSplat);
        description.inputRate = vk::VertexInputRate::eInstance;

        return description;
    }

    std::array<vk::VertexInputAttributeDescription, 4> get_splat_vertex_attribute_descriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 4> attributes;

        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format = vk::Format::eR32G32B32Sfloat;
        attributes[0].offset = offsetof(GpuSplat, position);

        attributes[1].binding = 0;
        attributes[1].location = 1;
        attributes[1].format = vk::Format::eR32G32B32A32Sfloat;
        attributes[1].offset = offsetof(GpuSplat, color);

        attributes[2].binding = 0;
        attributes[2].location = 2;
        attributes[2].format = vk::Format::eR32G32B32Sfloat;
        attributes[2].offset = offsetof(GpuSplat, logScale);

        attributes[3].binding = 0;
        attributes[3].location = 3;
        attributes[3].format = vk::Format::eR32G32B32A32Sfloat;
        attributes[3].offset = offsetof(GpuSplat, rotation);

        return attributes;
    }

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

    AllocatedBuffer upload_splat_data(
        const void* data,
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        const char* stagingName,
        const char* bufferName,
        VmaAllocator& allocator,
        vk::Device& device,
        vk::CommandPool commandPool,
        vk::Queue& queue
    ) {
        AllocatedBuffer stagingBuffer = create_buffer(
            allocator,
            size, 
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_MEMORY_USAGE_AUTO,
            (
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT
            ),
            stagingName
        );

        if(!stagingBuffer.buffer)
            return {};

        if(!write_buffer(allocator, stagingBuffer, data, size))
        {
            destroy_buffer(allocator, stagingBuffer);
            return {};
        }

        AllocatedBuffer gpuBuffer = create_buffer(
            allocator,
            size,
            (usage | vk::BufferUsageFlagBits::eTransferDst),
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            0,
            bufferName
        );

        if(!gpuBuffer.buffer)
        {
            destroy_buffer(allocator, stagingBuffer);
            return {};
        }

        if(!copy_buffer(
            stagingBuffer.buffer, gpuBuffer.buffer, size,
            device, queue, commandPool
        )) {
            destroy_buffer(allocator, gpuBuffer);
            destroy_buffer(allocator, stagingBuffer);

            return {};
        }

        destroy_buffer(allocator, stagingBuffer);

        return gpuBuffer;
    }

    std::vector<glm::vec4> pack_spherical_harmonics(const SplatCloud& cloud)
    {
        std::vector<glm::vec4> sphericalHarmonics;
        sphericalHarmonics.reserve(
            std::max<size_t>(cloud.sphericalHarmonics.size(), 1)
        );

        for(const glm::vec3& coefficient : cloud.sphericalHarmonics)
            sphericalHarmonics.emplace_back(coefficient, 0.0f);

        if(sphericalHarmonics.empty())
            sphericalHarmonics.emplace_back(0.0f);

        return sphericalHarmonics;
    }
}
