#include "renderer/resources/pipeline/PipelineLayout.hpp"

#include <vector>

#include "logging/Logger.hpp"

PipelineLayoutBuilder::PipelineLayoutBuilder(vk::Device& device)
{
    m_device = &device;
}

vk::PipelineLayout PipelineLayoutBuilder::build(
    const ShaderInterface& shaderInterface,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts =
        shaderInterface.get_descriptor_set_layouts();
    
    const std::vector<vk::PushConstantRange>& pushConstantRanges =
        shaderInterface.get_push_constant_ranges();
    
    vk::PipelineLayoutCreateInfo layoutInfo = {};

    layoutInfo.flags = vk::PipelineLayoutCreateFlagBits();
    layoutInfo.setLayoutCount = descriptorSetLayouts.size();
    layoutInfo.pSetLayouts = descriptorSetLayouts.data();
    layoutInfo.pushConstantRangeCount = pushConstantRanges.size();
    layoutInfo.pPushConstantRanges = pushConstantRanges.data();

    vk::ResultValue<vk::PipelineLayout> layoutAttempt = 
        m_device->createPipelineLayout(layoutInfo);

    if(layoutAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create pipeline layout");
        return vk::PipelineLayout();
    }

    logger->print("Created pipeline layout");

    vk::PipelineLayout handle = layoutAttempt.value;
    deletionQueue.push_back(
        [logger, handle] (vk::Device device)->void {
            device.destroyPipelineLayout(handle);
            logger->print("Deleted pipeline layout");
        }
    );

    return layoutAttempt.value;
}
