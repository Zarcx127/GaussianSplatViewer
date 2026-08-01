#include "renderer/core/Instance.hpp"

#include <vector>
#include <sstream>
#include <GLFW/glfw3.h>

#include "logging/Logger.hpp"

#include "backend/Utils.hpp"

using std::vector;

namespace 
{
    vector<const char*> get_required_extension_names(
        uint32_t& requiredExtensionCount, const char** extensionNames = nullptr, uint32_t extensionCount = 0
    );

    vector<const char*> get_required_layer_names(
        uint32_t& requiredLayerCount, const char** layerNames = nullptr, uint32_t layerCount = 0
    );

    bool supported_by_instance(const vector<const char*>& extensionNames, const vector<const char*>& layerNames);
}

vk::Instance make_instance(
    const char* applicationName, 
    std::deque<std::function<void(vk::Instance)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    uint32_t version = vk::enumerateInstanceVersion().value;
    logger->report_version_number(version);

    version &= ~(0xFFFU);

    vk::ApplicationInfo appInfo = vk::ApplicationInfo(applicationName, version, NULL, version, version);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    uint32_t enabledExtensionCount = 0;
    vector<const char*> enabledExtensionNames = get_required_extension_names(
        enabledExtensionCount, 
        glfwExtensions,
        glfwExtensionCount
    );

    uint32_t enabledLayerCount = 0;
    vector<const char*> enabledLayerNames = get_required_layer_names(enabledLayerCount);

    if(!supported_by_instance(enabledExtensionNames, enabledLayerNames))
        return nullptr;

    vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
        vk::InstanceCreateFlags(),
        &appInfo,
        enabledLayerCount,
        enabledLayerNames.data(),
        enabledExtensionCount,
        enabledExtensionNames.data()
    );

    vk::ResultValue<vk::Instance> instanceAttempt = vk::createInstance(createInfo);
    if(instanceAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create instance");
        return vk::Instance();
    }

    vk::Instance instance = instanceAttempt.value;

    deletionQueue.push_back([logger] (vk::Instance instance)->void{
        instance.destroy();
        logger->print("Deleted instance");
    });

    return instance;
}

namespace 
{
    vector<const char*> get_required_extension_names(
        uint32_t& requiredExtensionCount, 
        const char** extensionNames, 
        uint32_t extensionCount
    ) {
        Logger* logger = Logger::get_logger();

        vector<const char*> requiredExtensionNames(extensionCount);
        for(uint32_t i = 0; i < extensionCount; i++)
            requiredExtensionNames[i] = extensionNames[i];

        requiredExtensionCount = extensionCount;

        if(logger->is_enabled())
        {
            requiredExtensionCount++;
            requiredExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return requiredExtensionNames;
    }

    vector<const char*> get_required_layer_names(
        uint32_t& requiredLayerCount, 
        const char** layerNames, 
        uint32_t layerCount
    ) {
        Logger* logger = Logger::get_logger();

        vector<const char*> requiredLayerNames(layerCount);
        for(uint32_t i = 0; i < layerCount; i++)
            requiredLayerNames[i] = layerNames[i];

        requiredLayerCount = layerCount;

        if(logger->is_enabled())
        {
            requiredLayerCount++;
            requiredLayerNames.push_back("VK_LAYER_KHRONOS_validation");
        }

        return requiredLayerNames;
    }

    bool supported_by_instance(
        const vector<const char*>& extensionNames, 
        const vector<const char*>& layerNames
    ) {
        Logger* logger = Logger::get_logger();

        vector<vk::ExtensionProperties> supportedExtensions = 
            vk::enumerateInstanceExtensionProperties().value;
        
        vector<const char*> supportedExtensionsNames(supportedExtensions.size());
        for(int i = 0; i < supportedExtensions.size(); i++)
            supportedExtensionsNames[i] = supportedExtensions[i].extensionName;

        if(!utils::vector_compare(extensionNames, supportedExtensionsNames))
            return false;

        vector<vk::LayerProperties> supportedLayers =  
            vk::enumerateInstanceLayerProperties().value;
        
        vector<const char*> supportedLayerNames(supportedLayers.size());
        for(int i = 0; i < supportedLayers.size(); i++)
            supportedLayerNames[i] = supportedLayers[i].layerName;

        if(!utils::vector_compare(layerNames, supportedLayerNames))
            return false;
        
        return true;
    }
}
