#pragma once

#ifndef RENDERER_RESOURCES_PIPELINE_RENDER_FEATURE_PIPELINES_H
#define RENDERER_RESOURCES_PIPELINE_RENDER_FEATURE_PIPELINES_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

struct SplatEntryScanPipelines
{
    vk::Pipeline localScan {};
    vk::Pipeline blockScan {};
    vk::Pipeline addBlockOffsets {};
    vk::Pipeline finalize {};
};

struct SplatSortPipelines
{
    vk::Pipeline histogram {};
    vk::Pipeline histogramLocalScan {};
    vk::Pipeline histogramBlockScan {};
    vk::Pipeline histogramAddBlockOffsets {};
    vk::Pipeline bucketOffsetScan {};
    vk::Pipeline scatter {};
};

struct RenderFeaturePipelines
{
    vk::Pipeline background {};
    vk::Pipeline splatPreprocess {};
    vk::Pipeline splatTile {};
    vk::Pipeline splatTileRange {};
    vk::Pipeline splatTileRender {};

    SplatEntryScanPipelines splatEntryScan {};
    SplatSortPipelines splatSort{};
};

RenderFeaturePipelines build_render_feature_pipeline(
    vk::Device device,
    vk::PipelineLayout pipelineLayout,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

bool render_feature_pipelines_are_valid(
    const RenderFeaturePipelines& pipelines
);

#endif
