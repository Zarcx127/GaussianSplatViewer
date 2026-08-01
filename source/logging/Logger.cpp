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

#include "logging/Logger.hpp"

#include "BuildConfig.hpp"

using std::vector;

Logger* Logger::get_logger()
{
    static Logger logger;
    return &logger;
}

void Logger::set_mode(bool mode)
{
    m_enabled = build::enableLogging && mode;
}

#ifdef DEBUG

#include <iostream>

using std::cout;
using std::endl;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagBitsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT*,
    void* pUserData
);

void Logger::print(const char* msg)
{
    if(!m_enabled) return;
    
    cout << msg << endl;
}

void Logger::report_version_number(uint32_t version)
{
    if(!m_enabled) return;
    
    cout << "System can support vulkan Variant: " << vk::apiVersionVariant(version)
        << ", Major: " << vk::apiVersionMajor(version)
        << ", Minor: " << vk::apiVersionMinor(version)
        << ", Patch: " << vk::apiVersionPatch(version) << endl;
}

vk::DebugUtilsMessengerEXT Logger::make_debug_messenger(
    vk::Instance& instance, std::deque<std::function<void(vk::Instance)>>& deletionQueue
) {
    using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;
    
    vk::DebugUtilsMessengerCreateInfoEXT createInfo = vk::DebugUtilsMessengerCreateInfoEXT(
        vk::DebugUtilsMessengerCreateFlagsEXT(),
        Severity::eError | Severity::eInfo | Severity::eVerbose | Severity::eWarning,
        Type::eGeneral | Type::ePerformance | Type::eValidation, 
        reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback), 
        nullptr
    );
    
    vk::ResultValue<vk::DebugUtilsMessengerEXT> createResult  = 
        instance.createDebugUtilsMessengerEXT(createInfo);

    if(createResult.result != vk::Result::eSuccess)
    {
        cout << "Failed to create debug messenger" << endl;
        return vk::DebugUtilsMessengerEXT();
    }

    vk::DebugUtilsMessengerEXT messenger = createResult.value;
    deletionQueue.push_back(
        [this, messenger] (vk::Instance instance)->void {
            instance.destroyDebugUtilsMessengerEXT(messenger);
 
           if(m_enabled)
                cout << "Deleted debug messenger" << endl;
        }
    );

    return messenger;
}

void Logger::print_vector(const vector<const char*>& vec, const char* prefix)
{
    if(!m_enabled) return;

    for(const char* str : vec)
        cout << prefix << str << endl;
}

void Logger::log(const vk::PhysicalDevice& device)
{
    if(!m_enabled) return;

    vk::PhysicalDeviceProperties properties = device.getProperties();
    using Device = vk::PhysicalDeviceType;

    cout << "Device name: " << properties.deviceName << endl;
    cout << "Device Type: ";
    switch(properties.deviceType)
    {
        case Device::eCpu:
            cout << "CPU";
            break;
        
        case Device::eDiscreteGpu:
            cout << "Discrete GPU";
            break;
        
        case Device::eIntegratedGpu:
            cout << "Integrated GPU";
            break;
        
        case Device::eVirtualGpu:
            cout << "Virtual GPU";
            break;
        
        default:
            cout << "Other";
    }

    cout << endl;
}

void Logger::log(const vector<vk::QueueFamilyProperties>& queueFamilies, const char* prefix)
{
    if(!m_enabled) return;

    cout << prefix << "Number of queue families: " << queueFamilies.size() << endl; 
    for(uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        cout << prefix << "Queue Family " << i << " Supports: ";
        
        vk::QueueFlags queueFlags = queueFamilies[i].queueFlags;
        using Flag = vk::QueueFlagBits;         
        
        vector<const char*> supportedFunctions;
        
        if(queueFlags & Flag::eCompute)
            supportedFunctions.push_back("compute");

        if(queueFlags & Flag::eTransfer)
            supportedFunctions.push_back("transfer");

        if(queueFlags & Flag::eGraphics)
            supportedFunctions.push_back("graphics");
        
        if(queueFlags & Flag::eOpticalFlowNV)
            supportedFunctions.push_back("nvidia optical flow");

        if(queueFlags & Flag::eSparseBinding)
            supportedFunctions.push_back("sparse binding");

        if(queueFlags & Flag::eProtected)
            supportedFunctions.push_back("protected binding");

        if(queueFlags & Flag::eVideoDecodeKHR)
            supportedFunctions.push_back("video decode");
        
        if(queueFlags & Flag::eVideoEncodeKHR)
            supportedFunctions.push_back("video encode");

        for(uint32_t j = 0; j < supportedFunctions.size(); j++)
        {
            cout << supportedFunctions[j]
                << (((j+1) < supportedFunctions.size()) ? ", " : "");
        }

        cout << endl;
    }
}

void Logger::log(const vk::SurfaceCapabilitiesKHR& capabilities, const char* prefix)
{
    if(!m_enabled) return;

    cout << "Swapchain can support the following surface capabilities:" << endl;
    cout << prefix << "min image count: " << capabilities.minImageCount << endl;
    cout << prefix << "max image count: " << capabilities.maxImageCount << endl;
    
    cout << prefix << "current extent: " << endl; 
    log(capabilities.currentExtent, "\t\t");

    cout << prefix << "min supported extent: " << endl;
    log(capabilities.minImageExtent, "\t\t");

    cout << prefix << "max supported extent: " << endl;
    log(capabilities.maxImageExtent, "\t\t");

    cout << prefix << "max image array layers: " << capabilities.maxImageArrayLayers << endl;

    vector<const char*> stringList;

    stringList = parse_transform_bits(capabilities.currentTransform);
    cout << prefix << "supported transforms: " << endl;
    print_vector(stringList, "\t\t");

    stringList = parse_alpha_composite_bits(capabilities.supportedCompositeAlpha);
    cout << prefix << "supported alpha operation: " << endl;
    print_vector(stringList, "\t\t");
}

void Logger::log(const vk::Extent2D& extent, const char* prefix)
{
    if(!m_enabled) return;

    cout << prefix << "width: " << extent.width << endl;
    cout << prefix << "height: " << extent.height << endl;
}

void Logger::log(const vector<vk::SurfaceFormatKHR>& formats, const char* prefix)
{
    if(!m_enabled) return;

    for(vk::SurfaceFormatKHR supportedFormat : formats)
    {
        cout << prefix << "supported pixel format: "
            << vk::to_string(supportedFormat.format) << endl;

        cout << prefix << "supported color space: "
            << vk::to_string(supportedFormat.colorSpace) << endl;
    }
}

void Logger::log(const vector<vk::PresentModeKHR>& modes, const char* prefix)
{
    if(!m_enabled) return;

    for(vk::PresentModeKHR mode : modes)
        cout << prefix << vk::to_string(mode) << endl;
}

void Logger::log(const VmaAllocationInfo& info, const char* prefix)
{
    if(!m_enabled) return;

    cout << "-----" << info.pName << "-----" << endl;
    cout << prefix << "memory type: " << info.memoryType << endl;
    cout << prefix << "memory object: " << info.deviceMemory << endl;
    cout << prefix << "offset: " << info.offset << endl;
    cout << prefix << "size: " << info.size << endl;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT,
    VkDebugUtilsMessageTypeFlagBitsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*
) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << endl;

    return vk::False;
}


vector<const char*> Logger::parse_transform_bits(vk::SurfaceTransformFlagsKHR bits)
{
    vector<const char*> results;
    using Flags = vk::SurfaceTransformFlagBitsKHR;

    if(bits & Flags::eIdentity)
        results.push_back("identity");

    if(bits & Flags::eRotate90)
        results.push_back("90 degree Rotation");

    if(bits & Flags::eRotate180)
        results.push_back("180 degree Rotation");

    if(bits & Flags::eRotate270)
        results.push_back("270 degree Rotation");

    if(bits & Flags::eHorizontalMirror)
        results.push_back("horizontal Mirror");

    if(bits & Flags::eHorizontalMirrorRotate90)
        results.push_back("horizontal mirror, then 90 degree rotation");

    if(bits & Flags::eHorizontalMirrorRotate180)
        results.push_back("horizontal mirror, then 180 degree rotation");

    if(bits & Flags::eHorizontalMirrorRotate270)
        results.push_back("horizontal mirror, then 270 degree rotation");

    if(bits & Flags::eInherit)
        results.push_back("inherited");

    return results;
}

vector<const char*> Logger::parse_alpha_composite_bits(vk::CompositeAlphaFlagsKHR bits)
{
    vector<const char*> results;
    using Flags = vk::CompositeAlphaFlagBitsKHR;

    if(bits & Flags::eOpaque)
        results.push_back("opaque (alpha ignore)");

    if(bits & Flags::ePreMultiplied)
        results.push_back("pre multiplied (alpha expected to already be multiplied in image)");

    if(bits & Flags::ePostMultiplied)
        results.push_back("post multiplied (alpha will be applied during composition)");

    if(bits & Flags::eInherit)
        results.push_back("inherited");

    return results;
}

#else

void Logger::print(const char* msg) {}

void Logger::report_version_number(uint32_t version) {}

vk::DebugUtilsMessengerEXT Logger::make_debug_messenger(
    vk::Instance& instance, std::deque<std::function<void(vk::Instance)>>& deletionQueue
) {
    return vk::DebugUtilsMessengerEXT();
}

void Logger::print_vector(const std::vector<const char*>& vec, const char* prefix) {}

void Logger::log(const vk::PhysicalDevice& device) {}
void Logger::log(const std::vector<vk::QueueFamilyProperties>& queueFamily, const char* prefix) {}
void Logger::log(const vk::SurfaceCapabilitiesKHR& capabilities, const char* prefix) {}
void Logger::log(const vk::Extent2D& extent, const char* prefix) {}
void Logger::log(const std::vector<vk::SurfaceFormatKHR>& formats, const char* prefix) {}
void Logger::log(const std::vector<vk::PresentModeKHR>& modes, const char* prefix) {}
void Logger::log(const VmaAllocationInfo& info, const char* prefix) {}

vector<const char*> Logger::parse_transform_bits(vk::SurfaceTransformFlagsKHR bits) 
{
    return vector<const char*>{};
}

vector<const char*> Logger::parse_alpha_composite_bits(vk::CompositeAlphaFlagsKHR bits) 
{
    return vector<const char*>{};
}

#endif
