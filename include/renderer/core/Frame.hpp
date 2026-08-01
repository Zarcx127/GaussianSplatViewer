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

class Frame
{
public:
    Swapchain* swapchain { nullptr };
    std::vector<vk::ShaderEXT>* shaders { nullptr }; 

    vk::CommandBuffer commandBuffer;

    vk::Semaphore imageacquiredSemaphore;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    vk::Fence renderFinishedFence;
    
    Frame(
        Swapchain* swapchain, 
        vk::Device device, 
        std::vector<vk::ShaderEXT>* shaders, 
        vk::CommandBuffer& commandBuffer,
        const vk::DescriptorSetLayout* descriptorSetLayout,
        const vk::DescriptorPool* descriptorPool,
        const vk::PipelineLayout* pipelineLayout,
        const AllocatedImage* depthImage,
        Mesh* mesh, 
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

    void record_command_buffer(uint32_t imageIndex, const glm::mat4& mvp);

private:
    vk::RenderingInfoKHR m_renderingInfo {};
    vk::RenderingAttachmentInfoKHR m_colorAttachment {};
    vk::RenderingAttachmentInfoKHR m_depthAttachment {};
    
    const vk::DescriptorSetLayout* m_descriptorSetLayout { nullptr };
    const vk::DescriptorPool* m_descriptorPool { nullptr };
    const vk::PipelineLayout* m_pipelineLayout { nullptr };

    vk::DescriptorSet m_descriptorSet;

    Mesh* m_mesh;

    const AllocatedImage* m_depthImage { nullptr };
    
    void build_rendering_info();

    void build_color_attachment(uint32_t imageIndex);

    void build_depth_attachment();

    void initialize_render_state();
};

#endif
