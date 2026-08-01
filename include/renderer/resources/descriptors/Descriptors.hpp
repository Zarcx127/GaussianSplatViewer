#pragma once

#ifndef RENDERER_RESOURCES_DESCRIPTORS_DESCRIPTORS_H
#define RENDERER_RESOURCES_DESCRIPTORS_DESCRIPTORS_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

class DescriptorSetLayoutBuilder
{
public:
    DescriptorSetLayoutBuilder(vk::Device& device);

    vk::DescriptorSetLayout build(std::deque<std::function<void(vk::Device)>>& deletionQueue);

    void add_entry(vk::ShaderStageFlags stage, vk::DescriptorType type);

private:
    vk::Device* m_device;

    std::vector<vk::DescriptorSetLayoutBinding> m_layoutBindings;

    void reset();
};

class DescriptorPoolBuilder
{
public:
    DescriptorPoolBuilder(vk::Device& device);

    vk::DescriptorPool build(uint32_t descriptorSetCount, std::deque<std::function<void(vk::Device)>>& deletionQueue);

    void add_entry(vk::DescriptorType bindingType, uint32_t descriptorCount);

private:
    vk::Device* m_device;

    std::vector<vk::DescriptorPoolSize> m_poolSizes;
};

vk::DescriptorSet allocate_descriptor_set(
    vk::Device device, 
    const vk::DescriptorPool& descriptorPool, 
    const vk::DescriptorSetLayout& descriptorSetLayout
);

void write_storage_image_descriptor(
    vk::Device device, 
    vk::DescriptorSet descriptorSet,
    vk::ImageView imageView,
    vk::ImageLayout imageLayout,
    uint32_t binding = 0
);

#endif
