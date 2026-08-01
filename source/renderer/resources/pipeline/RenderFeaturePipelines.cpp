#include "renderer/resources/pipeline/RenderFeaturePipelines.hpp"

#include <vector>

#include "renderer/resources/shaders/shader.hpp"

#include "renderer/shaderPaths.hpp"

namespace
{
    struct ComputePipelineEntry
    {
        vk::Pipeline& pipeline;
        const char* 
        shaderPath;
    };
}

RenderFeaturePipelines build_render_feature_pipeline(
    vk::Device device,
    vk::PipelineLayout pipelineLayout,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    RenderFeaturePipelines pipelines = {};

    if(!device || !pipelineLayout)
        return pipelines;

    std::vector<ComputePipelineEntry> entries;
    entries.reserve(15);

    entries.push_back({
        pipelines.background, 
        shader::background
    });

    entries.push_back({
        pipelines.splatPreprocess, 
        shader::splatPreprocess
    });
    
    entries.push_back({
        pipelines.splatTile, 
        shader::splatTile
    });
    
    entries.push_back({
        pipelines.splatEntryScan.localScan, 
        shader::splatEntryLocalScan
    });

    entries.push_back({
        pipelines.splatEntryScan.blockScan, 
        shader::splatEntryBlockScan
    });

    entries.push_back({
        pipelines.splatEntryScan.addBlockOffsets, 
        shader::splatEntryAddBlockOffsets
    });

    entries.push_back({
        pipelines.splatEntryScan.finalize, 
        shader::splatEntryFinalize
    });

    entries.push_back({
        pipelines.splatSort.histogram, 
        shader::splatSortHistogram
    });

    entries.push_back({
        pipelines.splatSort.histogramLocalScan, 
        shader::splatSortHistogramLocalScan
    });

    entries.push_back({
        pipelines.splatSort.histogramBlockScan, 
        shader::splatSortHistogramBlockScan
    });

    entries.push_back({
        pipelines.splatSort.histogramAddBlockOffsets, 
        shader::splatSortHistogramAddBlockOffsets
    });

    entries.push_back({
        pipelines.splatSort.bucketOffsetScan,
        shader::splatSortBucketOffsetScan
    });

    entries.push_back({
        pipelines.splatSort.scatter,
        shader::splatSortScatter
    });

    entries.push_back({
        pipelines.splatTileRange,
        shader::splatTileRange
    });

    entries.push_back({
        pipelines.splatTileRender,
        shader::splatTileRender
    });

    for(const ComputePipelineEntry& entry : entries)
    {
        entry.pipeline = make_compute_pipeline(
            device, entry.
        shaderPath,
            pipelineLayout, deletionQueue
        );

        if(!entry.pipeline)
            return {};
    }

    return pipelines;
}

bool render_feature_pipelines_are_valid(
    const RenderFeaturePipelines& pipelines
) {
    return (
        pipelines.background &&
        pipelines.splatPreprocess &&
        pipelines.splatTile &&
        pipelines.splatEntryScan.localScan &&
        pipelines.splatEntryScan.blockScan &&
        pipelines.splatEntryScan.addBlockOffsets &&
        pipelines.splatEntryScan.finalize &&
        pipelines.splatSort.histogram &&
        pipelines.splatSort.histogramLocalScan &&
        pipelines.splatSort.histogramBlockScan &&
        pipelines.splatSort.histogramAddBlockOffsets &&
        pipelines.splatSort.bucketOffsetScan &&
        pipelines.splatSort.scatter &&
        pipelines.splatTileRange &&
        pipelines.splatTileRender
    );
}
