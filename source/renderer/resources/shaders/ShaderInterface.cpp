/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

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
