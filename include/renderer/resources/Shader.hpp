#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

class PipelineLayoutBuilder
{
public:
    PipelineLayoutBuilder(vk::Device& device);

    vk::PipelineLayout build(std::deque<std::function<void(vk::Device)>>& deletionQueue);

    void add(vk::DescriptorSetLayout descriptorSetLayout);

private:
    vk::Device* m_device;

    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;

    void reset();
};

std::vector<vk::ShaderEXT> make_shader_object(
    vk::Device logicalDevice, 
    const char* vertexFileName, 
    const char* fragmentFileName, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

#endif
