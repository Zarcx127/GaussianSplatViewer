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

#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include <deque>
#include <functional>
#include <filesystem>

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include "ViewerState.hpp"

#include "input/InputState.hpp"

#include "renderer/camera/EditorCamera.hpp"

#include "renderer/core/VulkanContext.hpp"

#include "renderer/resources/shaders/ShaderInterface.hpp"

#include "renderer/LoadingScreen.hpp"
#include "renderer/RenderFeatures.hpp"
#include "renderer/RenderResources.hpp"

class Engine
{
public:
    Engine(GLFWwindow* window, const std::filesystem::path& splatPath);

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void render_loop(
        ViewerState& state, 
        const std::function<InputState()>& getInput
    );
    
    const char* load_error() const;
    
    ~Engine();
    
private:
    GLFWwindow* m_window { nullptr };

    std::filesystem::path m_splatPath {};
    const char* m_loadError { nullptr };

    std::deque<std::function<void(vk::Device)>> m_interfaceDeletionQueue;
    
    VulkanContext m_vulkanContext;

    ShaderInterface m_renderInterface;

    vk::DescriptorSetLayout m_descriptorSetLayout {};
    vk::DescriptorSetLayout m_sphericalHarmonicDescriptorSetLayout {};
    vk::DescriptorSetLayout m_splatFrameDescriptorSetLayout {};

    vk::PipelineLayout m_pipelineLayout {};

    LoadingScreen m_loadingScreen;
    RenderFeatures m_renderFeatures;
    RenderResources m_renderResources;

    EditorCamera m_camera;

    double m_currentTime { 0.0 };
    double m_lastTime { 0.0 };
    double m_lastFrameTime { 0.0 };

    uint32_t m_currFrame { 0 };
    uint32_t m_numFrames { 0 };
    uint32_t m_splatEntryCapacity { 0 };

    bool m_rebuildSwapchain { false };
    bool m_initialized { false };

    enum class DrawResult
    {
        Success,
        Skipped,
        NeedsSwapchainRebuild,
        FatalError
    };

    bool init(uint32_t width, uint32_t height, ViewerState& state);

    DrawResult draw(uint32_t width, uint32_t height, const InputState& input);
    void update_timing(ViewerState& state);

    bool init_render_features(ViewerState& state);
    void destroy_render_features();

    bool init_core_render_resources(uint32_t width, uint32_t height);
    bool init_splat_render_resources();
    void destroy_render_resources();

    RenderFeaturesContext make_render_features_context();
    RenderResourcesContext make_render_resources_context();

    void shutdown();
};

#endif
