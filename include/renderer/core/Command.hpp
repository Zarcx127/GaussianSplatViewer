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

#ifndef RENDERER_CORE_COMMAND_H
#define RENDERER_CORE_COMMAND_H

#include <deque>
#include <functional>

#include <vulkan/vulkan.hpp>

vk::CommandPool make_command_pool(
    vk::Device device, 
    uint32_t graphicsQueueFamilyIndex, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

vk::CommandBuffer make_command_buffer(
    vk::Device device, 
    vk::CommandPool commandPool,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
);

bool immediate_submit(
    vk::Device device,
    vk::CommandPool commandPool,
    vk::Queue queue,
    const std::function<void(vk::CommandBuffer)>& function
);

#endif
