#include "renderer/resources/splats/SplatFrameDescriptors.hpp"

#include "renderer/resources/descriptors/Descriptors.hpp"

namespace
{
    vk::ShaderStageFlags get_splat_frame_binding_stages(SplatFrameBinding binding);

    const AllocatedBuffer* get_splat_frame_binding_buffer(
        SplatFrameBinding binding,
        const SplatBuffer& splatBuffer,
        const SplatFrameResources& resources
    );
}

vk::DescriptorSetLayout build_splat_frame_descriptor_set_layout(
    vk::Device& device,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    DescriptorSetLayoutBuilder builder(device);
    for(uint32_t index = 0; index < SPLAT_FRAME_BINDING_COUNT; index++)
    {
        SplatFrameBinding binding = static_cast<SplatFrameBinding>(index);

        builder.add_entry(
            index,
            get_splat_frame_binding_stages(binding),
            vk::DescriptorType::eStorageBuffer
        );
    }

    return builder.build(deletionQueue);
}

bool write_splat_frame_descriptor_set(
    vk::Device device,
    vk::DescriptorSet descriptorSet,
    const SplatBuffer& splatBuffer,
    const SplatFrameResources& resources
) {
    for(uint32_t index = 0; index < SPLAT_FRAME_BINDING_COUNT; index++)
    {
        SplatFrameBinding binding = static_cast<SplatFrameBinding>(index);
        const AllocatedBuffer* buffer = get_splat_frame_binding_buffer(
            binding, splatBuffer, resources
        );

        if(!buffer || !buffer->buffer)
            return false;

        write_storage_buffer_descriptor(
            device, descriptorSet,
            buffer->buffer, 0, buffer->size, index
        );
    }

    return true;
}

namespace
{
    vk::ShaderStageFlags get_splat_frame_binding_stages(SplatFrameBinding binding)
    {
        if(
            (binding == SplatFrameBinding::ProjectedSplats) ||
            (binding == SplatFrameBinding::VisibleSplatIndices) ||
            (binding == SplatFrameBinding::EntrySplatIndicesA) ||
            (binding == SplatFrameBinding::EntrySplatIndicesB)
        ) {
            return (
                vk::ShaderStageFlagBits::eCompute |
                vk::ShaderStageFlagBits::eVertex
            );
        }

        return vk::ShaderStageFlagBits::eCompute;
    }

    const AllocatedBuffer* get_splat_frame_binding_buffer(
        SplatFrameBinding binding,
        const SplatBuffer& splatBuffer,
        const SplatFrameResources& resources
    ) {
        using Binding = SplatFrameBinding;
        switch(binding)
        {
            case Binding::SplatData: 
                return &splatBuffer.buffer;
            
            case Binding::ProjectedSplats: 
                return &resources.projectedSplats;
            
            case Binding::VisibleSplatIndices: 
                return &resources.visibleSplatIndices;
            
            case Binding::EntryKeysA:
                return &resources.entryKeys[0];

            case Binding::EntryKeysB:
                return &resources.entryKeys[1];

            case Binding::EntrySplatIndicesA:
                return &resources.entrySplatIndices[0];

            case Binding::EntrySplatIndicesB:
                return &resources.entrySplatIndices[1];

            case Binding::Counters:
                return &resources.counters;

            case Binding::DrawCommand:
                return &resources.drawCommand;

            case Binding::TileRanges:
                return &resources.tileRanges;

            case Binding::EntryRanges:
                return &resources.entryRanges;

            case Binding::EntryScanBlockSums:
                return &resources.entryScanBlockSums;

            case Binding::SortRadixHistograms:
                return &resources.sort.radixHistograms;

            case Binding::SortRadixScanBlockSums:
                return &resources.sort.radixScanBlockSums;

            case Binding::SortRadixBucketOffsets:
                return &resources.sort.radixBucketOffsets;
        }

        return nullptr;
    }
}
