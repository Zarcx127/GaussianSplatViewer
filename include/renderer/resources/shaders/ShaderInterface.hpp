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

#pragma once

#ifndef RENDERER_RESOURCES_SHADERS_SHADER_INTERFACE_H
#define RENDERER_RESOURCES_SHADERS_SHADER_INTERFACE_H

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
