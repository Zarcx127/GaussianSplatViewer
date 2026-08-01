#pragma once

#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

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

    void add_entry(vk::DescriptorType bindingType);

private:
    vk::Device* m_device;

    std::vector<vk::DescriptorPoolSize> m_poolSizes;
};

vk::DescriptorSet allocate_descriptor_set(
    vk::Device device, 
    vk::DescriptorPool& descriptorPool, 
    vk::DescriptorSetLayout& descriptorSetLayout
);

#endif
