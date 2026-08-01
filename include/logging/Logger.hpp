#pragma once

#ifndef LOGGER_H
#define LOGGER_H

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
    bool is_enabled();

    void print(const char* msg);

    void report_version_number(uint32_t version);

    vk::DebugUtilsMessengerEXT make_debug_messenger(
        vk::Instance& instance, std::deque<std::function<void(vk::Instance)>>& deletionQueue
    );
    
    void print_list(const char** list, uint32_t count, const char* prefix = "\t");
    
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
    std::vector<const char*> parse_image_usage_bits(vk::ImageUsageFlags bits);
};

#endif
