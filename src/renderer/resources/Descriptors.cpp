#include "renderer/resources/Descriptors.hpp"

#ifdef DESCRIPTORS_H

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
    layoutInfo.bindingCount = m_layoutBinding.size();
    layoutInfo.pBindings = m_layoutBinding.data();

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
    deletionQueue.push_back([logger, handle] (vk::Device device)->void{
        device.destroyDescriptorSetLayout(handle);
        logger->print("Deleted descriptor set layout");
    });

    return layoutAttempt.value;
}

void DescriptorSetLayoutBuilder::add_entry(
    vk::ShaderStageFlags stage, 
    vk::DescriptorType type
) {
    vk::DescriptorSetLayoutBinding entry = {};

    entry.binding = m_layoutBinding.size();
    entry.descriptorCount = 1;
    entry.descriptorType = type;
    entry.stageFlags = stage;

    m_layoutBinding.push_back(entry);
}

void DescriptorSetLayoutBuilder::reset()
{
    m_layoutBinding.clear();
}

#endif
