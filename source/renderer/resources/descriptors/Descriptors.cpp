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
    add_entry(
        static_cast<uint32_t>(m_layoutBindings.size()),
        stage, type
    );
}

void DescriptorSetLayoutBuilder::add_entry(
    uint32_t binding,
    vk::ShaderStageFlags stage,
    vk::DescriptorType type
) {
    vk::DescriptorSetLayoutBinding entry = {};

    entry.binding = binding;
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

vk::DescriptorPool DescriptorPoolBuilder::build(
    uint32_t descriptorSetCount, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
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

void DescriptorPoolBuilder::add_entry(
    vk::DescriptorType bindingType, 
    uint32_t descriptorCount
) {
    vk::DescriptorPoolSize poolSize = {};

    poolSize.descriptorCount = descriptorCount;
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

void write_storage_image_descriptor(
    vk::Device device, 
    vk::DescriptorSet descriptorSet,
    vk::ImageView imageView,
    vk::ImageLayout imageLayout,
    uint32_t binding
) {
    vk::DescriptorImageInfo imageInfo = {};

    imageInfo.sampler = nullptr;
    imageInfo.imageView = imageView;
    imageInfo.imageLayout = imageLayout;

    vk::WriteDescriptorSet write = {};

    write.dstSet = descriptorSet;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = vk::DescriptorType::eStorageImage;
    write.pImageInfo = &imageInfo;

    device.updateDescriptorSets(1, &write, 0, nullptr);
}

void write_storage_buffer_descriptor(
    vk::Device device,
    vk::DescriptorSet descriptorSet,
    vk::Buffer buffer,
    vk::DeviceSize offset,
    vk::DeviceSize range,
    uint32_t binding
) {
    vk::DescriptorBufferInfo bufferInfo = {};

    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    vk::WriteDescriptorSet write = {};

    write.dstSet = descriptorSet;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorCount = 1;
    write.descriptorType = vk::DescriptorType::eStorageBuffer;
    write.pBufferInfo = &bufferInfo;

    device.updateDescriptorSets(1, &write, 0, nullptr);
}
