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

#ifndef LOGGING_LOGGER_H
#define LOGGING_LOGGER_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

#include <vma/vk_mem_alloc.h>

class Logger
{    
public:
    static Logger* get_logger();

    void set_mode(bool mode);

    void print(const char* msg);

    void report_version_number(uint32_t version);

    vk::DebugUtilsMessengerEXT make_debug_messenger(
        vk::Instance& instance, std::deque<std::function<void(vk::Instance)>>& deletionQueue
    );
    
    void print_vector(const std::vector<const char*>& vec, const char* prefix = "\t");
    
    void log(const vk::PhysicalDevice& device);
    void log(const std::vector<vk::QueueFamilyProperties>& queueFamily, const char* prefix = "\t");
    void log(const vk::SurfaceCapabilitiesKHR& capabilities, const char* prefix = "\t");
    void log(const vk::Extent2D& extent, const char* prefix = "\t");
    void log(const std::vector<vk::SurfaceFormatKHR>& formats, const char* prefix = "\t");
    void log(const std::vector<vk::PresentModeKHR>& modes, const char* prefix = "\t");
    void log(const VmaAllocationInfo& info, const char* prefix = "\t");

private:
    bool m_enabled { false };
    
    std::vector<const char*> parse_transform_bits(vk::SurfaceTransformFlagsKHR bits);
    std::vector<const char*> parse_alpha_composite_bits(vk::CompositeAlphaFlagsKHR bits);
};

#endif
