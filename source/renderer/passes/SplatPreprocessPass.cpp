#include "renderer/passes/SplatPreprocessPass.hpp"

#include <array>
#include <vector>

#include "renderer/core/Synchronization.hpp"

namespace 
{
    constexpr uint32_t SPLAT_PREPROCESS_LOCAL_SIZE = 256;
    constexpr uint32_t SPLAT_QUAD_VERTEX_COUNT = 4;

    constexpr uint32_t RESET_ENTRIES_COUNT = 2;
    constexpr uint32_t OUTPUT_ENTRIES_COUNT = 5;

    struct BufferBarrierEntry
    {
        const AllocatedBuffer* buffer;
        vk::AccessFlags destinationAccess;
    };

    template<std::size_t ENTRY_COUNT>
    void build_buffer_memory_barriers(
        std::vector<BufferBarrierEntry>& entries,
        std::array<vk::BufferMemoryBarrier, ENTRY_COUNT>& barriers,
        vk::AccessFlags sourceAccess
    );
}

void record_splat_preprocess_pass(
    vk::CommandBuffer commandBuffer,
    vk::Pipeline pipeline,
    vk::PipelineLayout pipelineLayout,
    vk::DescriptorSet sphericalHarmonicDescriptorSet,
    vk::DescriptorSet splatFrameDescriptorSet,
    const SplatFrameResources& resources
) {
    GpuSplatCounters counters = {};

    commandBuffer.updateBuffer(
        resources.counters.buffer,
        0,
        sizeof(GpuSplatCounters),
        &counters
    );

    vk::DrawIndirectCommand drawCommand = {};
    drawCommand.vertexCount = SPLAT_QUAD_VERTEX_COUNT;

    commandBuffer.updateBuffer(
        resources.drawCommand.buffer,
        0,
        sizeof(vk::DrawIndirectCommand),
        &drawCommand
    );

    constexpr vk::AccessFlags SHADER_READ = vk::AccessFlagBits::eShaderRead;
    constexpr vk::AccessFlags SHADER_WRITE = vk::AccessFlagBits::eShaderWrite;
    constexpr vk::AccessFlags SHADER_READ_WRITE = (SHADER_READ | SHADER_WRITE);
    constexpr vk::AccessFlags INDIRECT_COMMAND_READ = 
        vk::AccessFlagBits::eIndirectCommandRead;

    std::vector<BufferBarrierEntry> resetEntries;
    resetEntries.reserve(RESET_ENTRIES_COUNT);

    resetEntries.push_back({&resources.counters, SHADER_READ_WRITE});
    resetEntries.push_back({&resources.drawCommand, SHADER_READ_WRITE});

    std::array<vk::BufferMemoryBarrier, RESET_ENTRIES_COUNT> resetBarriers;
    build_buffer_memory_barriers(
        resetEntries, resetBarriers,
        vk::AccessFlagBits::eTransferWrite
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::DependencyFlags(),
        nullptr,
        resetBarriers,
        nullptr
    );

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline);

    std::array<vk::DescriptorSet, 2> descriptorSets = {
        sphericalHarmonicDescriptorSet,
        splatFrameDescriptorSet
    };

    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        pipelineLayout,
        1,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(),
        0,
        nullptr
    );

    uint32_t groupCount = (
        (resources.splatCapacity + SPLAT_PREPROCESS_LOCAL_SIZE - 1) / 
        SPLAT_PREPROCESS_LOCAL_SIZE
    );
    
    commandBuffer.dispatch(groupCount, 1, 1);

    std::vector<BufferBarrierEntry> outputEntries;
    outputEntries.reserve(OUTPUT_ENTRIES_COUNT);

    outputEntries.push_back({&resources.projectedSplats, SHADER_READ});
    outputEntries.push_back({&resources.visibleSplatIndices, SHADER_READ});
    outputEntries.push_back({&resources.counters, SHADER_READ_WRITE});
    outputEntries.push_back({&resources.drawCommand, INDIRECT_COMMAND_READ});
    outputEntries.push_back({&resources.entryRanges, SHADER_READ_WRITE});

    std::array<vk::BufferMemoryBarrier, OUTPUT_ENTRIES_COUNT> outputBarriers;
    build_buffer_memory_barriers(
        outputEntries, outputBarriers,
        vk::AccessFlagBits::eShaderWrite
    );

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eComputeShader,
        (
            vk::PipelineStageFlagBits::eComputeShader |
            vk::PipelineStageFlagBits::eVertexShader |
            vk::PipelineStageFlagBits::eDrawIndirect
        ),
        vk::DependencyFlags(),
        nullptr,
        outputBarriers,
        nullptr
    );
}

namespace
{
    template<std::size_t ENTRY_COUNT>
    void build_buffer_memory_barriers(
        std::vector<BufferBarrierEntry>& entries,
        std::array<vk::BufferMemoryBarrier, ENTRY_COUNT>& barriers,
        vk::AccessFlags sourceAccess
    ) {
        for(uint32_t index = 0; index < ENTRY_COUNT; index++)
        {
            const BufferBarrierEntry& entry = entries[index];

            barriers[index] = make_buffer_memory_barrier(
                entry.buffer->buffer,
                entry.buffer->size,
                sourceAccess,
                entry.destinationAccess
            );
        }
    }
}