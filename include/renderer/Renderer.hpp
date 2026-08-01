#pragma once

#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

#include <GLFW/glfw3.h>

#include "AppState.hpp"

#include "logging/Logger.hpp"

#include "input/InputState.hpp"

#include "renderer/camera/EditorCamera.hpp"

#include "renderer/core/VulkanContext.hpp"

#include "renderer/resources/shaders/ShaderInterface.hpp"

#include "renderer/RenderFeatures.hpp"
#include "renderer/RenderResources.hpp"

class Engine
{
public:
    Engine(GLFWwindow* window);

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;

    void render_loop(
        AppState& state, const std::function<InputState()>& getInput
    );
    
    ~Engine();
    
private:
    GLFWwindow* m_window { nullptr };
    Logger* m_logger { nullptr };

    std::deque<std::function<void(vk::Device)>> m_interfaceDeletionQueue;
    
    VulkanContext m_vulkanContext;

    ShaderInterface m_renderInterface;

    vk::DescriptorSetLayout m_descriptorSetLayout {};
    vk::DescriptorSetLayout m_sphericalHarmonicDescriptorSetLayout {};
    vk::DescriptorSetLayout m_splatFrameDescriptorSetLayout {};

    vk::PipelineLayout m_pipelineLayout {};

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

    bool init(uint32_t width, uint32_t height);

    DrawResult draw(uint32_t width, uint32_t height, const InputState& input);
    void update_timing(AppState& state);

    bool init_render_features();
    void destroy_render_features();

    bool init_render_resources(uint32_t width, uint32_t height);
    void destroy_render_resources();

    RenderFeaturesContext make_render_features_context();
    RenderResourcesContext make_render_resources_context();

    void shutdown();
};

#endif
