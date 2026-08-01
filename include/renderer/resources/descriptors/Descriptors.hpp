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

#ifndef RENDERER_RESOURCES_DESCRIPTORS_DESCRIPTORS_H
#define RENDERER_RESOURCES_DESCRIPTORS_DESCRIPTORS_H

#include <deque>
#include <vector>
#include <cstdint>
#include <functional>

#include <vulkan/vulkan.hpp>

class DescriptorSetLayoutBuilder
{
public:
    DescriptorSetLayoutBuilder(vk::Device& device);

    vk::DescriptorSetLayout build(
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

    void add_entry(vk::ShaderStageFlags stage, vk::DescriptorType type);
    
    void add_entry(
        uint32_t binding,
        vk::ShaderStageFlags stage,
        vk::DescriptorType type
    );

private:
    vk::Device* m_device;

    std::vector<vk::DescriptorSetLayoutBinding> m_layoutBindings;

    void reset();
};

class DescriptorPoolBuilder
{
public:
    DescriptorPoolBuilder(vk::Device& device);

    vk::DescriptorPool build(
        uint32_t descriptorSetCount, 
        std::deque<std::function<void(vk::Device)>>& deletionQueue
    );

    void add_entry(
        vk::DescriptorType bindingType, 
        uint32_t descriptorCount
    );

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

void write_storage_buffer_descriptor(
    vk::Device device,
    vk::DescriptorSet descriptorSet,
    vk::Buffer buffer,
    vk::DeviceSize offset,
    vk::DeviceSize range,
    uint32_t binding = 0
);

#endif
