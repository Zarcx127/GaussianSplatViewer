#include "renderer/core/Device.hpp"

#include <vector>
#include <algorithm>

#include "logging/Logger.hpp"

#include "backend/Utils.hpp"

#include "renderer/core/Instance.hpp"

using std::vector;

namespace 
{
    uint8_t get_device_type_score(const vk::PhysicalDevice& device);

    uint64_t get_device_vram(const vk::PhysicalDevice& device);
}

bool supports(const vk::PhysicalDevice& device, vector<const char*> requestedExtensions) 
{
    vector<vk::ExtensionProperties> supportedExtensions = device.enumerateDeviceExtensionProperties().value;
    vector<const char*> supportedExtensionNames(supportedExtensions.size());
    for(uint32_t i = 0; i < supportedExtensions.size(); i++)
        supportedExtensionNames[i] = supportedExtensions[i].extensionName;

    if(!utils::vector_compare(requestedExtensions, supportedExtensionNames))
        return false;

    return true;
}

bool is_suitable(const vk::PhysicalDevice device)
{
    Logger* logger = Logger::get_logger();

    vector<const char*> requestedExtensions;
    requestedExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    if(!supports(device, requestedExtensions))
    {
        logger->print("Device can't support all requested extensions");
        return false;
    }

    logger->print("Device can support all requested extensions");
    return true;
}

vk::PhysicalDevice choose_physical_device(const vk::Instance instance)
{
    Logger* logger = Logger::get_logger();

    vector<vk::PhysicalDevice> availableDevices, suitableDevices;
    availableDevices = instance.enumeratePhysicalDevices().value;
    for(vk::PhysicalDevice device : availableDevices)
        if(is_suitable(device))
            suitableDevices.push_back(device);

    if(suitableDevices.empty())
    {
        logger->print("No suitable device found");
        return vk::PhysicalDevice();
    }

    std::sort(
        suitableDevices.begin(), suitableDevices.end(), 
        [] (const vk::PhysicalDevice& devA, const vk::PhysicalDevice& devB)->bool{
            uint8_t devAType = get_device_type_score(devA);
            uint8_t devBType = get_device_type_score(devB);

            if(devAType != devBType)
                return (devAType > devBType);

            uint64_t devAVRam = get_device_vram(devA);
            uint64_t devBVRam = get_device_vram(devB);

            return (devAVRam > devBVRam);
        }
    );

    logger->log(suitableDevices[0]);
    return suitableDevices[0];
}

uint32_t find_queue_family_index(
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface, 
    vk::QueueFlags queueType
){
    Logger* logger = Logger::get_logger();

    vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
    logger->log(queueFamilies, "");

    for(uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        vk::QueueFamilyProperties queueFamily = queueFamilies[i];

        bool canPresent = true;
        if(surface)
            if(physicalDevice.getSurfaceSupportKHR(i, surface).value != vk::True)
                canPresent = false;

        bool supported = false;
        if(queueFamily.queueFlags & queueType)
            supported = true;

        if(canPresent && supported)
            return i;
    }

    return UINT32_MAX;
}

vk::Device create_logical_device(
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    uint32_t graphicsIndex = find_queue_family_index(physicalDevice, surface, vk::QueueFlagBits::eGraphics);
    float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueInfo = vk::DeviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(), graphicsIndex, 1, &queuePriority
    );

    vk::PhysicalDeviceFeatures deviceFeatures = {};
    vk::PhysicalDeviceShaderObjectFeaturesEXT shaderFeatures = {};
    vk::PhysicalDeviceVulkan13Features vulkan13Features = {};
    
    shaderFeatures.sType = vk::StructureType::ePhysicalDeviceShaderObjectFeaturesEXT;
    vulkan13Features.sType = vk::StructureType::ePhysicalDeviceVulkan13Features;
    
    deviceFeatures.samplerAnisotropy = vk::True;
    shaderFeatures.shaderObject = vk::True;
    vulkan13Features.dynamicRendering = vk::True;
    vulkan13Features.synchronization2 = vk::True;
    
    vk::PhysicalDeviceFeatures2 featureChain = {};
    featureChain.sType  = vk::StructureType::ePhysicalDeviceFeatures2;
    featureChain.features = deviceFeatures;
    
    featureChain.pNext  = &vulkan13Features;
    vulkan13Features.pNext = &shaderFeatures;

    vector<const char*> enabledLayers;
    
    if(logger->is_enabled())
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

    vector<const char*> enabledExtensions = {
        "VK_KHR_swapchain",
        "VK_EXT_shader_object",
        "VK_KHR_dynamic_rendering",
        "VK_KHR_maintenance4"
    };

    vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
        vk::DeviceCreateFlags(), 
        1, 
        &queueInfo,
        enabledLayers.size(),
        enabledLayers.data(), 
        enabledExtensions.size(),
        enabledExtensions.data(),
        nullptr
    );

    deviceInfo.pNext = &featureChain;

    vk::ResultValueType<vk::Device>::type logicalDevice = physicalDevice.createDevice(deviceInfo);

    if(logicalDevice.result != vk::Result::eSuccess)
    {
        logger->print("Logical device creation failed");
        return vk::Device();
    }
    
    logger->print("GPU has been successfully abstracted");
    deletionQueue.push_back(
        [logger] (vk::Device device)->void {
            (void) device.waitIdle();
            device.destroy();

            logger->print("Deleted logical device");
        }
    );

    return logicalDevice.value;
}

namespace
{
    uint8_t get_device_type_score(const vk::PhysicalDevice& device)
    {
        vk::PhysicalDeviceProperties properties = device.getProperties();
        
        using Device = vk::PhysicalDeviceType;
        switch(properties.deviceType)
        {
            case Device::eDiscreteGpu:
                return 5; 
                break;
            
            case Device::eIntegratedGpu:
                return 4; 
                break;
            
            case Device::eVirtualGpu:
                return 3; 
                break;
            
            case Device::eCpu:
                return 2; 
                break;
        }

        return 1;
    }

    uint64_t get_device_vram(const vk::PhysicalDevice& device)
    {
        vk::PhysicalDeviceMemoryProperties memory = device.getMemoryProperties();
        constexpr uint64_t GIGABYTE = 1024ULL * 1024ULL * 1024ULL;

        uint64_t vram = 0;
        for(const vk::MemoryHeap& heap : memory.memoryHeaps)
            if(heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                vram += heap.size;

        return (vram / GIGABYTE);
    }
}
