#pragma once

#ifndef SHADER_INTERFACE_H
#define SHADER_INTERFACE_H

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.hpp>

class ShaderInterface
{
public:
    void add_descriptor_set_layout(vk::DescriptorSetLayout descriptorSetLayout);
    void add_push_constant_range(
        vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size
    );

    const std::vector<vk::DescriptorSetLayout>& get_descriptor_set_layouts() const;
    const std::vector<vk::PushConstantRange>& get_push_constant_ranges() const;

private:
    std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
    std::vector<vk::PushConstantRange> m_pushConstantRanges;
};

#endif
