#pragma once

#ifndef RENDERER_RENDER_RESOURCES_H
#define RENDERER_RENDER_RESOURCES_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "renderer/core/Frame.hpp"
#include "renderer/core/Swapchain.hpp"

#include "renderer/resources/images/AllocatedImage.hpp"

#include "renderer/resources/shaders/ShaderInterface.hpp"

#include "renderer/resources/splats/Splat.hpp"

struct RenderResourcesContext
{
    vk::PhysicalDevice physicalDevice;
    vk::Device logicalDevice;
    vk::SurfaceKHR surface;

    VmaAllocator allocator { nullptr };

    vk::CommandPool commandPool;
    vk::Queue graphicsQueue;

    vk::DescriptorSetLayout splatFrameDescriptorSetLayout;
    vk::PipelineLayout pipelineLayout;

    const ShaderInterface& renderInterface;
};

class RenderResources
{
public:
    RenderResources() = default;

    RenderResources(const RenderResources&) = delete;
    RenderResources& operator=(const RenderResources&) = delete;

    RenderResources(RenderResources&&) = delete;
    RenderResources& operator=(RenderResources&&) = delete;

    
    bool build(
        RenderResourcesContext& context,
        uint32_t width,
        uint32_t height,
        uint32_t framesInFlight,
        const SplatBuffer& splatBuffer,
        uint32_t entryCapacity
    );

    void destroy(RenderResourcesContext& context);

    vk::SwapchainKHR swapchain_handle() const;
    vk::Extent2D extent() const;

    vk::Image color_image(uint32_t imageIndex) const;
    vk::ImageView color_image_view(uint32_t imageIndex) const;

    uint32_t color_image_count() const;
    uint32_t color_image_view_count() const;

    const AllocatedImage& depth_image() const;

    vk::Pipeline splat_gaussian_pipeline() const;

    vk::DescriptorSet swapchain_storage_descriptor_set(uint32_t imageIndex) const;
    uint32_t swapchain_storage_descriptor_set_count() const;

    Frame& frame(uint32_t index);
    uint32_t frame_count() const;
    
    vk::Fence& image_in_flight(uint32_t imageIndex);
    uint32_t image_in_flight_count() const;

private:
    std::deque<std::function<void(vk::Device)>> m_deletionQueue;
    std::deque<std::function<void(VmaAllocator)>> m_vmaDeletionQueue;

    Swapchain m_swapchain;
    AllocatedImage m_depthImage;

    vk::DescriptorPool m_descriptorPool {};
    std::vector<vk::DescriptorSet> m_swapchainImageDescriptorSets;

    vk::Pipeline m_splatGaussianPipeline {};

    std::vector<Frame> m_frames;
    std::vector<vk::Fence> m_imagesInFlight;
};

bool render_resources_context_is_valid(
    const RenderResourcesContext& context
);

#endif
