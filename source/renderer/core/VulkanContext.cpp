#include "renderer/core/VulkanContext.hpp"

#include "logging/Logger.hpp"

#include "renderer/core/Instance.hpp"
#include "renderer/core/Surface.hpp"
#include "renderer/core/Device.hpp"
#include "renderer/core/Command.hpp"

#include "renderer/resources/Allocator.hpp"

bool VulkanContext::build(GLFWwindow* window, const char* applicationName)
{
    Logger* logger = Logger::get_logger();

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
    
    m_instance = make_instance(applicationName, m_instanceDeletionQueue);
    if(!m_instance)
    {
        destroy();
        return false;
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

#ifdef DEBUG
    
    m_debugMessenger = logger->make_debug_messenger(m_instance, m_instanceDeletionQueue);
    if(!m_debugMessenger && logger->is_enabled())
    {
        destroy();
        return false;
    }

#endif

    m_surface = make_vulkan_surface(m_instance, window, m_instanceDeletionQueue);
    if(!m_surface)
    {
        destroy();
        return false;
    }
    
    PhysicalDeviceSelection physicalDeviceSelection = 
        choose_physical_device(m_instance, m_surface);

    m_physicalDevice = physicalDeviceSelection.device;
    if(!m_physicalDevice)
    {
        destroy();
        return false;
    }

    m_graphicsQueueFamilyIndex = physicalDeviceSelection.queueFamilyIndex;
    if(m_graphicsQueueFamilyIndex == UINT32_MAX)
    {
        destroy();
        return false;
    }
    
    m_logicalDevice = create_logical_device(
        m_physicalDevice, m_graphicsQueueFamilyIndex, m_deviceDeletionQueue
    );
    
    if(!m_logicalDevice)
    {
        destroy();
        return false;
    }

    m_graphicsQueue = m_logicalDevice.getQueue(m_graphicsQueueFamilyIndex, 0);
    m_commandPool = make_command_pool(
        m_logicalDevice, m_graphicsQueueFamilyIndex, m_deviceDeletionQueue
    );

    if(!m_commandPool)
    {
        destroy();
        return false;
    }

    m_allocator = make_vma_allocator(m_instance, m_physicalDevice, m_logicalDevice);
    if(!m_allocator)
    {
        destroy();
        return false;
    }
    
    return true;
}

void VulkanContext::destroy()
{
    if(m_graphicsQueue)
        (void) m_graphicsQueue.waitIdle();

    if(m_allocator) 
        vmaDestroyAllocator(m_allocator);

    while(!m_deviceDeletionQueue.empty())
    {
        if(m_logicalDevice)
            (m_deviceDeletionQueue.back())(m_logicalDevice);
        
        m_deviceDeletionQueue.pop_back();
    }
    
    while(!m_instanceDeletionQueue.empty())
    {
        if(m_instance)
            (m_instanceDeletionQueue.back())(m_instance);

        m_instanceDeletionQueue.pop_back();
    }

    m_allocator = nullptr;
    m_commandPool = vk::CommandPool();
    m_graphicsQueue = vk::Queue();
    m_graphicsQueueFamilyIndex = UINT32_MAX;
    m_logicalDevice = vk::Device();
    m_physicalDevice = vk::PhysicalDevice();
    m_surface = vk::SurfaceKHR();
    m_debugMessenger = vk::DebugUtilsMessengerEXT();
    m_instance = vk::Instance();
}

vk::Instance VulkanContext::instance() const
{
    return m_instance;
}

vk::PhysicalDevice VulkanContext::physical_device() const
{
    return m_physicalDevice;
}

vk::Device VulkanContext::logical_device() const
{
    return m_logicalDevice;
}

vk::Device& VulkanContext::logical_device_ref()
{
    return m_logicalDevice;
}

VmaAllocator VulkanContext::allocator() const
{
    return m_allocator;
}

vk::SurfaceKHR VulkanContext::surface() const
{
    return m_surface;
}

uint32_t VulkanContext::graphics_queue_family_index() const
{
    return m_graphicsQueueFamilyIndex;
}

vk::Queue VulkanContext::graphics_queue() const
{
    return m_graphicsQueue;
}

vk::CommandPool VulkanContext::command_pool() const
{
    return m_commandPool;
}
