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

#ifndef RENDERER_LOADING_SCREEN_H
#define RENDERER_LOADING_SCREEN_H

#include <deque>
#include <vector>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.hpp>

#include "renderer/core/VulkanContext.hpp"

#include "renderer/RenderResources.hpp"

struct alignas(16) LoadingScreenPushConstant
{
    float width;
    float height;
    float time;
};

class LoadingScreen
{
public:
    LoadingScreen() = default;

    LoadingScreen(const LoadingScreen&) = delete;
    LoadingScreen& operator=(const LoadingScreen&) = delete;

    LoadingScreen(LoadingScreen&&) = delete;
    LoadingScreen& operator=(LoadingScreen&&) = delete;

    bool build(
        VulkanContext& vulkanContext,
        RenderResources& renderResources,
        vk::PipelineLayout pipelineLayout
    );

    bool draw(float time);

    void destroy();

    ~LoadingScreen();

private:
    VulkanContext* m_vulkanContext { nullptr };
    RenderResources* m_renderResources { nullptr };

    std::deque<std::function<void(vk::Device)>> m_deletionQueue;

    vk::PipelineLayout m_pipelineLayout {};
    vk::Pipeline m_pipeline {};

    vk::CommandBuffer m_commandBuffer {};

    vk::Semaphore m_imageAcquiredSemaphore {};
    std::vector<vk::Semaphore> m_renderFinishedSemaphores {};
    vk::Fence m_renderFinishedFence {};

    bool build_pipeline();
    bool record(uint32_t imageIndex, float time);
};

#endif
