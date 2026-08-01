#include "renderer/core/Device.hpp"

#include <vector>
#include <algorithm>

#include "backend/Utils.hpp"

#include "logging/Logger.hpp"

#include "renderer/core/Instance.hpp"

#include "BuildConfig.hpp"

namespace 
{
    bool supports_extensions(
        const vk::PhysicalDevice& device, 
        std::vector<const char*> requestedExtensions
    );

    PhysicalDeviceSelection get_suitable_device(
        vk::PhysicalDevice device, 
        vk::SurfaceKHR surface
    );

    uint8_t get_device_type_score(const vk::PhysicalDevice& device);

    uint64_t get_device_vram(const vk::PhysicalDevice& device);

    std::vector<const char*> get_required_device_extensions();

    bool supports_dynamic_rendering(vk::PhysicalDevice device);

    bool supports_required_surface_features(
        vk::PhysicalDevice device,
        vk::SurfaceKHR surface
    );
}

PhysicalDeviceSelection choose_physical_device(vk::Instance instance, vk::SurfaceKHR surface)
{
    Logger* logger = Logger::get_logger();

    std::vector<vk::PhysicalDevice> availableDevices;
    availableDevices = instance.enumeratePhysicalDevices().value;

    std::vector<PhysicalDeviceSelection> suitableDevices;
    for(vk::PhysicalDevice device : availableDevices)
    {
        PhysicalDeviceSelection selection = get_suitable_device(device, surface);
        if(selection.device)
            suitableDevices.push_back(selection);
    }

    if(suitableDevices.empty())
    {
        logger->print("No suitable device found");
        return {};
    }

    std::sort(
        suitableDevices.begin(), suitableDevices.end(), 
        [] (
            const PhysicalDeviceSelection& devA, 
            const PhysicalDeviceSelection& devB
        )->bool{
            uint8_t devAType = get_device_type_score(devA.device);
            uint8_t devBType = get_device_type_score(devB.device);

            if(devAType != devBType)
                return (devAType > devBType);

            uint64_t devAVRam = get_device_vram(devA.device);
            uint64_t devBVRam = get_device_vram(devB.device);

            return (devAVRam > devBVRam);
        }
    );

    logger->log(suitableDevices[0].device);
    return suitableDevices[0];
}

uint32_t find_queue_family_index(
    vk::PhysicalDevice physicalDevice, 
    vk::SurfaceKHR surface, 
    vk::QueueFlags queueType
){
    Logger* logger = Logger::get_logger();

    std::vector<vk::QueueFamilyProperties> queueFamilies = 
        physicalDevice.getQueueFamilyProperties();
    
    logger->log(queueFamilies, "");

    for(uint32_t i = 0; i < queueFamilies.size(); i++)
    {
        vk::QueueFamilyProperties queueFamily = queueFamilies[i];

        bool canPresent = true;
        if(surface)
            if(physicalDevice.getSurfaceSupportKHR(i, surface).value != vk::True)
                canPresent = false;

        bool supported = false;
        if((queueFamily.queueFlags & queueType) == queueType)
            supported = true;

        if(canPresent && supported)
            return i;
    }

    return UINT32_MAX;
}

vk::Device create_logical_device(
    vk::PhysicalDevice physicalDevice, 
    uint32_t queueFamilyIndex,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueInfo = vk::DeviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(), queueFamilyIndex, 1, &queuePriority
    );

    vk::PhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = vk::True;

    vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
    dynamicRenderingFeatures.dynamicRendering = vk::True;
    dynamicRenderingFeatures.sType = 
        vk::StructureType::ePhysicalDeviceDynamicRenderingFeaturesKHR;

    vk::PhysicalDeviceFeatures2 featureChain = {};
    featureChain.sType  = vk::StructureType::ePhysicalDeviceFeatures2;
    featureChain.features = deviceFeatures;
    featureChain.pNext  = &dynamicRenderingFeatures;

    std::vector<const char*> enabledLayers;
    
    if(build::enableValidation)
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");

    std::vector<const char*> enabledExtensions = get_required_device_extensions();

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

    vk::ResultValueType<vk::Device>::type logicalDevice = 
        physicalDevice.createDevice(deviceInfo);

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
    bool supports_extensions(
        const vk::PhysicalDevice& device, 
        std::vector<const char*> requestedExtensions
    ) {
        std::vector<vk::ExtensionProperties> supportedExtensions = 
            device.enumerateDeviceExtensionProperties().value;
        
        std::vector<const char*> supportedExtensionNames(supportedExtensions.size());
        for(uint32_t i = 0; i < supportedExtensions.size(); i++)
            supportedExtensionNames[i] = supportedExtensions[i].extensionName;

        if(!utils::vector_compare(requestedExtensions, supportedExtensionNames))
            return false;

        return true;
    }

    PhysicalDeviceSelection get_suitable_device(
        vk::PhysicalDevice device, 
        vk::SurfaceKHR surface
    ) {
        Logger* logger = Logger::get_logger();

        std::vector<const char*> requestedExtensions = get_required_device_extensions();
        if(!supports_extensions(device, requestedExtensions))
        {
            logger->print("Device can't support all requested extensions");
            return {};
        }

        if(!supports_dynamic_rendering(device))
        {
            logger->print("Device does not support dynamic rendering");
            return {};
        }

        uint32_t renderQueueFamilyIndex = find_queue_family_index(
            device, 
            surface,
            vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute
        );

        if(renderQueueFamilyIndex == UINT32_MAX)
        {
            logger->print("Device has no graphics + compute + present queue");
            return {};
        }

        if(!supports_required_surface_features(device, surface))
        {
            logger->print("Device surface support is not enough for this renderer");
            return {};
        }

        logger->print("Device can support all requested extensions");
        return {device, renderQueueFamilyIndex};
    }

    uint8_t get_device_type_score(const vk::PhysicalDevice& device)
    {
        vk::PhysicalDeviceProperties properties = device.getProperties();
        
        using Device = vk::PhysicalDeviceType;
        switch(properties.deviceType)
        {
            case Device::eDiscreteGpu: return 5;
            case Device::eIntegratedGpu: return 4; 
            case Device::eVirtualGpu: return 3; 
            case Device::eCpu: return 2; 
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

    std::vector<const char*> get_required_device_extensions()
    {
        return {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };
    }

    bool supports_dynamic_rendering(vk::PhysicalDevice device)
    {
        vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
        dynamicRenderingFeatures.sType = 
            vk::StructureType::ePhysicalDeviceDynamicRenderingFeaturesKHR;

        vk::PhysicalDeviceFeatures2 features = {};
        features.sType = vk::StructureType::ePhysicalDeviceFeatures2;
        features.pNext = &dynamicRenderingFeatures;

        device.getFeatures2(&features);

        return (dynamicRenderingFeatures.dynamicRendering == vk::True);
    }

    bool supports_required_surface_features(
        vk::PhysicalDevice device, 
        vk::SurfaceKHR surface
    ) {
        vk::SurfaceCapabilitiesKHR capabilities =
            device.getSurfaceCapabilitiesKHR(surface).value;

        if(!(capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eStorage)) 
            return false;

        std::vector<vk::SurfaceFormatKHR> formats =
            device.getSurfaceFormatsKHR(surface).value;

        if(formats.empty()) 
            return false;

        bool hasRequiredFormat = false;
        for(const vk::SurfaceFormatKHR& format : formats)
        {
            if(
                (format.format == vk::Format::eR8G8B8A8Unorm) &&
                (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            ) {
                hasRequiredFormat = true;
                break;
            }
        }

        if(!hasRequiredFormat) 
            return false;

        std::vector<vk::PresentModeKHR> presentModes = 
            device.getSurfacePresentModesKHR(surface).value;

        if(presentModes.empty())
            return false;
        
        return true;
    }
}
