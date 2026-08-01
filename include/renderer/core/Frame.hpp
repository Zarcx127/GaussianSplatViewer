#pragma once

#ifndef FRAME_H
#define FRAME_H

#include <deque>
#include <vector>
#include <functional>

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "factory/MeshFactory.hpp"

#include "renderer/core/Swapchain.hpp"

#include "renderer/resources/images/AllocatedImage.hpp"

struct FramePushConstant
{
    glm::mat4 mvp;
    glm::mat4 invView;
    glm::mat4 invProj;
    glm::vec4 cameraPos;
};

class Frame
{
public:
    Swapchain* swapchain { nullptr };
    std::vector<vk::ShaderEXT>* shaders { nullptr }; 

    vk::CommandBuffer commandBuffer;

    vk::Semaphore imageAcquiredSemaphore;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    vk::Fence renderFinishedFence;
    
    Frame(
        Swapchain* swapchain, 
        vk::Device device, 
        std::vector<vk::ShaderEXT>* shaders, 
        const vk::ShaderEXT* computeShader,
        vk::CommandBuffer& commandBuffer,
        const std::vector<vk::DescriptorSet>* swapchainImageDescriptorSets,
        const vk::PipelineLayout* pipelineLayout,
        const AllocatedImage* depthImage,
        Mesh* mesh, 
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

    void record_command_buffer(
        uint32_t imageIndex, const FramePushConstant& pushConstants
    );

private:
    vk::RenderingInfoKHR m_renderingInfo {};
    vk::RenderingAttachmentInfoKHR m_colorAttachment {};
    vk::RenderingAttachmentInfoKHR m_depthAttachment {};
    
    const vk::ShaderEXT* m_computeShader { nullptr };
    const std::vector<vk::DescriptorSet>* m_swapchainImageDescriptorSets { nullptr };

    const vk::PipelineLayout* m_pipelineLayout { nullptr };

    Mesh* m_mesh;

    const AllocatedImage* m_depthImage { nullptr };
    
    void build_rendering_info();

    void build_color_attachment(uint32_t imageIndex);

    void build_depth_attachment();

    void initialize_render_state();

    void record_compute_background(uint32_t imageIndex);
};

#endif
