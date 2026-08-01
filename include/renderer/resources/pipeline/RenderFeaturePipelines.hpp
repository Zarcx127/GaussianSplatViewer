/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

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
