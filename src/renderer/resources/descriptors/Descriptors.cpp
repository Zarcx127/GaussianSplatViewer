#include "renderer/resources/descriptors/Descriptors.hpp"

#include "logging/Logger.hpp"

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(vk::Device& device)
{
    m_device = &device;
}

vk::DescriptorSetLayout DescriptorSetLayoutBuilder::build(
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::DescriptorSetLayoutCreateInfo layoutInfo = {};

    layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits();
    layoutInfo.bindingCount = m_layoutBindings.size();
    layoutInfo.pBindings = m_layoutBindings.data();

    vk::ResultValue<vk::DescriptorSetLayout> layoutAttempt = 
        m_device->createDescriptorSetLayout(layoutInfo);

    if(layoutAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create descriptor set layout");
        return vk::DescriptorSetLayout();
    }

    logger->print("Created descriptor set layout");
    reset();

    vk::DescriptorSetLayout handle = layoutAttempt.value;
    deletionQueue.push_back(
        [logger, handle] (vk::Device device)->void {
            device.destroyDescriptorSetLayout(handle);
            logger->print("Deleted descriptor set layout");
        }
    );

    return layoutAttempt.value;
}

void DescriptorSetLayoutBuilder::add_entry(
    vk::ShaderStageFlags stage, 
    vk::DescriptorType type
) {
    vk::DescriptorSetLayoutBinding entry = {};

    entry.binding = m_layoutBindings.size();
    entry.descriptorCount = 1;
    entry.descriptorType = type;
    entry.stageFlags = stage;

    m_layoutBindings.push_back(entry);
}

void DescriptorSetLayoutBuilder::reset()
{
    m_layoutBindings.clear();
}

DescriptorPoolBuilder::DescriptorPoolBuilder(vk::Device& device)
{
    m_device = &device;
}

vk::DescriptorPool DescriptorPoolBuilder::build(uint32_t descriptorSetCount, std::deque<std::function<void(vk::Device)>>& deletionQueue)
{
    Logger* logger = Logger::get_logger();
    vk::DescriptorPoolCreateInfo poolInfo = {};

    poolInfo.flags = vk::DescriptorPoolCreateFlagBits();
    poolInfo.maxSets = descriptorSetCount;
    poolInfo.poolSizeCount = m_poolSizes.size();
    poolInfo.pPoolSizes = m_poolSizes.data();

    vk::ResultValue<vk::DescriptorPool> descriptorPoolAttempt =
        m_device->createDescriptorPool(poolInfo);

    if(descriptorPoolAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create descriptor pool");
        return vk::DescriptorPool();
    }

    logger->print("Created descriptor pool");

    vk::DescriptorPool descriptorPool = descriptorPoolAttempt.value;
    deletionQueue.push_back(
        [logger, descriptorPool] (vk::Device device)->void {
            device.destroy(descriptorPool);
            logger->print("Deleted descriptor pool");
        }
    );

    return descriptorPool;
}

void DescriptorPoolBuilder::add_entry(vk::DescriptorType bindingType)
{
    vk::DescriptorPoolSize poolSize = {};

    poolSize.descriptorCount = 3;
    poolSize.type = bindingType;
    
    m_poolSizes.push_back(poolSize);
}

vk::DescriptorSet allocate_descriptor_set(
    vk::Device device, 
    const vk::DescriptorPool& descriptorPool, 
    const vk::DescriptorSetLayout& descriptorSetLayout
) {
    Logger* logger = Logger::get_logger();

    vk::DescriptorSetAllocateInfo allocationInfo = {};
    allocationInfo.descriptorPool = descriptorPool;
    allocationInfo.descriptorSetCount = 1;
    allocationInfo.pSetLayouts = &descriptorSetLayout;

    vk::ResultValue<std::vector<vk::DescriptorSet>> descriptorSetAttempt =
        device.allocateDescriptorSets(allocationInfo);

    if(descriptorSetAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to allocate descriptor set");
        return vk::DescriptorSet();
    }

    logger->print("Allocated descriptor set");

    return descriptorSetAttempt.value[0];
}
