#include "renderer/resources/shaders/ShaderInterface.hpp"

void ShaderInterface::add_descriptor_set_layout(vk::DescriptorSetLayout descriptorSetLayout)
{
    m_descriptorSetLayouts.push_back(descriptorSetLayout);
}

void ShaderInterface::add_push_constant_range(
    vk::ShaderStageFlags stageFlags, uint32_t offset, uint32_t size
) {
    vk::PushConstantRange range = {};

    range.stageFlags = stageFlags;
    range.offset = offset;
    range.size = size;

    m_pushConstantRanges.push_back(range);
}

const std::vector<vk::DescriptorSetLayout>& ShaderInterface::get_descriptor_set_layouts() const
{
    return m_descriptorSetLayouts;
}

const std::vector<vk::PushConstantRange>& ShaderInterface::get_push_constant_ranges() const
{
    return m_pushConstantRanges;
}
