#include "factory/MeshFactory.hpp"

#include <cstddef>

#include "renderer/resources/buffers/Buffer.hpp"

vk::VertexInputBindingDescription get_mesh_vertex_binding_description()
{
    vk::VertexInputBindingDescription description = {};

    description.binding = 0;
    description.stride = sizeof(MeshVertex);
    description.inputRate = vk::VertexInputRate::eVertex;

    return description;
}

std::array<vk::VertexInputAttributeDescription, 2> get_mesh_vertex_attribute_descriptions()
{
    std::array<vk::VertexInputAttributeDescription, 2> attributes;

    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = vk::Format::eR32G32B32Sfloat;
    attributes[0].offset = offsetof(MeshVertex, pos);

    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = vk::Format::eR32G32B32Sfloat;
    attributes[1].offset = offsetof(MeshVertex, color);

    return attributes;
}

GraphicsPipelineConfig get_mesh_pipeline_config()
{
    vk::VertexInputBindingDescription meshBindingDescription = 
        get_mesh_vertex_binding_description();

    std::array<vk::VertexInputAttributeDescription, 2> meshAttributeDescriptions =
        get_mesh_vertex_attribute_descriptions();

    GraphicsPipelineConfig meshPipelineConfig = {};

    meshPipelineConfig.vertexBindingDescriptions.push_back(meshBindingDescription);
    for(const vk::VertexInputAttributeDescription& attribute : meshAttributeDescriptions)
        meshPipelineConfig.vertexAttributeDescriptions.push_back(attribute);

    meshPipelineConfig.topology = vk::PrimitiveTopology::eTriangleList;

    meshPipelineConfig.cullMode = vk::CullModeFlagBits::eNone;
    meshPipelineConfig.frontFace = vk::FrontFace::eCounterClockwise;

    meshPipelineConfig.depthTest = true;
    meshPipelineConfig.depthWrite = true;
    meshPipelineConfig.depthCompareOp = vk::CompareOp::eLess;

    meshPipelineConfig.colorBlendAttachment.blendEnable = vk::False;
    meshPipelineConfig.colorBlendAttachment.colorWriteMask = (
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA 
    );

    return meshPipelineConfig;
}

Mesh build_cube(
    VmaAllocator& allocator, 
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
) {
    Mesh mesh = {};
    MeshVertex vertices[] = {
        // top, red
        {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}},
        {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}},
        {{-0.5f,  0.5f,  0.5f}, {1, 0, 0}},

        // bottom, green
        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 1, 0}},

        // left, blue
        {{-0.5f, -0.5f, -0.5f}, {0, 0, 1}},
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}},
        {{-0.5f,  0.5f, -0.5f}, {0, 0, 1}},

        // right, yellow
        {{ 0.5f, -0.5f,  0.5f}, {1, 1, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {1, 1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 1, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {1, 1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 1, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 1, 0}},

        // front, pink
        {{-0.5f,  0.5f,  0.5f}, {1, 0, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {1, 0, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 1}},
        {{-0.5f,  0.5f, -0.5f}, {1, 0, 1}},

        // back, cyan
        {{-0.5f, -0.5f, -0.5f}, {0, 1, 1}},
        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 1, 1}},
        {{-0.5f, -0.5f, -0.5f}, {0, 1, 1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 1, 1}},
        {{-0.5f, -0.5f,  0.5f}, {0, 1, 1}},
    };

    const vk::DeviceSize ALLOC_SIZE = sizeof(vertices);

    AllocatedBuffer stagingBuffer = create_buffer(
        allocator,
        ALLOC_SIZE,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_AUTO,
        (
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
            VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT
        ),
        "Cube Staging Buffer"
    );

    if(!stagingBuffer.buffer) 
        return {};

    if(!write_buffer(allocator, stagingBuffer, vertices, ALLOC_SIZE))
    {
        destroy_buffer(allocator, stagingBuffer);
        return {};
    }

    AllocatedBuffer vertexBuffer = create_buffer(
        allocator,
        ALLOC_SIZE,
        (
            vk::BufferUsageFlagBits::eVertexBuffer |
            vk::BufferUsageFlagBits::eTransferDst
        ),
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        0,
        "Cube MeshVertex Buffer"
    );

    if(!vertexBuffer.buffer)
    {
        destroy_buffer(allocator, stagingBuffer);
        return {};
    }

    bool copySuccessful = copy_buffer(
        stagingBuffer.buffer, vertexBuffer.buffer, 
        ALLOC_SIZE, device, queue, commandPool
    );

    if(!copySuccessful)
    {
        destroy_buffer(allocator, vertexBuffer);
        destroy_buffer(allocator, stagingBuffer);

        return {};
    }

    mesh.vertexBuffer = vertexBuffer;
    mesh.vertexBufferOffset = 0;
    mesh.vertexCount = static_cast<uint32_t>(
        sizeof(vertices) / sizeof(vertices[0])
    );

    destroy_buffer(allocator, stagingBuffer);

    deletionQueue.push_back(
        [mesh] (VmaAllocator allocator) mutable->void {
            destroy_buffer(allocator, mesh.vertexBuffer);
        }
    );

    return mesh;
}
