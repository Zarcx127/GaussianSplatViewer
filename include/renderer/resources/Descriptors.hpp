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

    std::vector<vk::DescriptorSetLayoutBinding> m_layoutBinding;

    void reset();
};

#endif
