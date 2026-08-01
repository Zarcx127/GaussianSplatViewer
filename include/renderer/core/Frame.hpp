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

class Frame
{
public:
    Swapchain* swapchain { nullptr };
    std::vector<vk::ShaderEXT>* shaders { nullptr }; 

    vk::CommandBuffer commandBuffer;

    vk::Semaphore imageAquiredSemaphore;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    vk::Fence renderFinishedFence;
    
    Frame(
        Swapchain* swapchain, 
        vk::Device device, 
        std::vector<vk::ShaderEXT>* shaders, 
        vk::CommandBuffer& commandBuffer,
        vk::DescriptorSetLayout* descriptorSetLayout,
        vk::DescriptorPool* descriptorPool,
        vk::PipelineLayout* pipelineLayout,
        Mesh* mesh,
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

    void record_command_buffer(uint32_t imageIndex, const glm::mat4& mvp);

private:
    vk::RenderingInfoKHR m_renderingInfo {};
    vk::RenderingAttachmentInfoKHR m_colorAttachment {};
    
    vk::DescriptorSetLayout* m_descriptorSetLayout { nullptr };
    vk::DescriptorPool* m_descriptorPool { nullptr };
    vk::PipelineLayout* m_pipelineLayout { nullptr };
    vk::DescriptorSet m_descriptorSet;

    Mesh* m_mesh;
    
    void build_rendering_info();

    void build_color_attachment(uint32_t imageIndex);

    void initialize_render_state();
};

#endif
