#include "factory/MeshFactory.hpp"

#include "logging/Logger.hpp"

#include "renderer/resources/Buffer.hpp"

vk::VertexInputBindingDescription2EXT get_binding_description()
{
    vk::VertexInputBindingDescription2EXT description = {};

    description.binding = 0;
    description.stride = sizeof(Vertex);
    description.inputRate = vk::VertexInputRate::eVertex;
    description.divisor = 1;

    return description;
}

std::vector<vk::VertexInputAttributeDescription2EXT> get_attribute_descriptions()
{
    std::vector<vk::VertexInputAttributeDescription2EXT> attributes(2);

    attributes[0].binding = 0;
    attributes[0].location = 0;
    attributes[0].format = vk::Format::eR32G32B32Sfloat;
    attributes[0].offset = offsetof(Vertex, pos);

    attributes[1].binding = 0;
    attributes[1].location = 1;
    attributes[1].format = vk::Format::eR32G32B32Sfloat;
    attributes[1].offset = offsetof(Vertex, color);

    return attributes;
}

// to delete

// Mesh build_triangle(
//     VmaAllocator& allocator, 
//     vk::Device& device,
//     vk::CommandPool& commandPool,
//     vk::Queue& queue,
//     std::deque<std::function<void(VmaAllocator)>>& deletionQueue
// ) {
//     Logger* logger = Logger::get_logger();
//     const uint32_t ALLOC_SIZE = (3 * sizeof(Vertex));

//     Mesh mesh;
//     Vertex vertices[3] = {
//         {{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
//         {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
//         {{0.0f, -0.5f}, {0.0f, 0.0f, 1.0f}}
//     };

//     VkBuffer stagingBuffer, vertexBuffer;
//     VmaAllocation stagingAllocation, vertexAllocation;
//     VmaAllocationInfo stagingInfo, vertexInfo;

//     vk::BufferCreateInfo bufferInfo = {};
    
//     bufferInfo.flags = vk::BufferCreateFlags();
//     bufferInfo.size = ALLOC_SIZE;
//     bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;

//     VkBufferCreateInfo bufferInfoHandle = bufferInfo;
//     VmaAllocationCreateInfo allocationInfo = {};

//     allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
//     allocationInfo.flags = (
//         VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
//         | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT
//     );
 
//     vmaCreateBuffer(
//         allocator, &bufferInfoHandle, &allocationInfo, &stagingBuffer, &stagingAllocation, &stagingInfo
//     );

//     vmaSetAllocationName(allocator, stagingAllocation, "Staging Buffer");
//     vmaGetAllocationInfo(allocator, stagingAllocation, &stagingInfo);

//     logger->log(stagingInfo);

//     void* dst;
//     vmaMapMemory(allocator, stagingAllocation, &dst);
//     memcpy(dst, vertices, ALLOC_SIZE);
//     vmaUnmapMemory(allocator, stagingAllocation);

//     bufferInfo.usage = (
//         vk::BufferUsageFlagBits::eVertexBuffer 
//         | vk::BufferUsageFlagBits::eTransferDst
//     );

//     bufferInfoHandle = bufferInfo;

//     allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
//     allocationInfo.flags = 0;

//     vmaCreateBuffer(
//         allocator, &bufferInfoHandle, &allocationInfo, &vertexBuffer, &vertexAllocation, &vertexInfo
//     );

//     vmaSetAllocationName(allocator, vertexAllocation, "Vertex Buffer");
//     vmaGetAllocationInfo(allocator, vertexAllocation, &vertexInfo);

//     logger->log(vertexInfo);

//     copy_buffer(stagingBuffer, stagingInfo, vertexBuffer, vertexInfo, ALLOC_SIZE, device, queue, commandPool);

//     mesh.buffer = vertexBuffer;
//     mesh.allocation = vertexAllocation;
//     mesh.offset = vertexInfo.offset;
//     mesh.numOfVertices = 3;

//     deletionQueue.push_back([stagingBuffer, stagingAllocation] (VmaAllocator allocator)->void{
//         vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
//     });

//     deletionQueue.push_back([mesh] (VmaAllocator allocator)->void{
//         vmaDestroyBuffer(allocator, mesh.buffer, mesh.allocation);
//     });

//     return mesh;
// }

//
///

Mesh build_cube(
    VmaAllocator& allocator, 
    vk::Device& device,
    vk::CommandPool& commandPool,
    vk::Queue& queue,
    std::deque<std::function<void(VmaAllocator)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    Mesh mesh;
    Vertex vertices[] = {
        // front
        {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {1, 0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}},
        {{-0.5f, -0.5f,  0.5f}, {1, 0, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 0}},
        {{-0.5f,  0.5f,  0.5f}, {1, 0, 0}},

        // back
        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 0}},
        {{-0.5f, -0.5f, -0.5f}, {0, 1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 1, 0}},

        // left
        {{-0.5f, -0.5f, -0.5f}, {0, 0, 1}},
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}},
        {{-0.5f, -0.5f, -0.5f}, {0, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}},
        {{-0.5f,  0.5f, -0.5f}, {0, 0, 1}},

        // right
        {{ 0.5f, -0.5f,  0.5f}, {1, 1, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {1, 1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 1, 0}},
        {{ 0.5f, -0.5f,  0.5f}, {1, 1, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 1, 0}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 1, 0}},

        // top
        {{-0.5f,  0.5f,  0.5f}, {1, 0, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {1, 0, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 1}},
        {{-0.5f,  0.5f,  0.5f}, {1, 0, 1}},
        {{ 0.5f,  0.5f, -0.5f}, {1, 0, 1}},
        {{-0.5f,  0.5f, -0.5f}, {1, 0, 1}},

        // bottom
        {{-0.5f, -0.5f, -0.5f}, {0, 1, 1}},
        {{ 0.5f, -0.5f, -0.5f}, {0, 1, 1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 1, 1}},
        {{-0.5f, -0.5f, -0.5f}, {0, 1, 1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 1, 1}},
        {{-0.5f, -0.5f,  0.5f}, {0, 1, 1}},
    };

    const vk::DeviceSize ALLOC_SIZE = sizeof(vertices);

    VkBuffer stagingBuffer, vertexBuffer;
    VmaAllocation stagingAllocation, vertexAllocation;
    VmaAllocationInfo stagingInfo, vertexInfo;

    vk::BufferCreateInfo bufferInfo = {};
    
    bufferInfo.flags = vk::BufferCreateFlags();
    bufferInfo.size = ALLOC_SIZE;
    bufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;

    VkBufferCreateInfo bufferInfoHandle = bufferInfo;
    VmaAllocationCreateInfo allocationInfo = {};

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocationInfo.flags = (
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        | VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT
    );
 
    vmaCreateBuffer(
        allocator, &bufferInfoHandle, &allocationInfo, &stagingBuffer, &stagingAllocation, &stagingInfo
    );

    vmaSetAllocationName(allocator, stagingAllocation, "Staging Buffer");
    vmaGetAllocationInfo(allocator, stagingAllocation, &stagingInfo);

    logger->log(stagingInfo);

    void* dst;
    vmaMapMemory(allocator, stagingAllocation, &dst);
    memcpy(dst, vertices, ALLOC_SIZE);
    vmaUnmapMemory(allocator, stagingAllocation);

    bufferInfo.usage = (
        vk::BufferUsageFlagBits::eVertexBuffer 
        | vk::BufferUsageFlagBits::eTransferDst
    );

    bufferInfoHandle = bufferInfo;

    allocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocationInfo.flags = 0;

    vmaCreateBuffer(
        allocator, &bufferInfoHandle, &allocationInfo, &vertexBuffer, &vertexAllocation, &vertexInfo
    );

    vmaSetAllocationName(allocator, vertexAllocation, "Vertex Buffer");
    vmaGetAllocationInfo(allocator, vertexAllocation, &vertexInfo);

    logger->log(vertexInfo);

    copy_buffer(stagingBuffer, stagingInfo, vertexBuffer, vertexInfo, ALLOC_SIZE, device, queue, commandPool);

    mesh.buffer = vertexBuffer;
    mesh.allocation = vertexAllocation;
    mesh.offset = 0;
    mesh.numOfVertices = static_cast<uint32_t>(
        sizeof(vertices) / sizeof(vertices[0])
    );

    deletionQueue.push_back([stagingBuffer, stagingAllocation] (VmaAllocator allocator)->void{
        vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
    });

    deletionQueue.push_back([mesh] (VmaAllocator allocator)->void{
        vmaDestroyBuffer(allocator, mesh.buffer, mesh.allocation);
    });

    return mesh;
}

