#include "renderer/resources/Shader.hpp"

#ifdef SHADER_H

#include "logging/Logger.hpp"

#include "backend/utils.hpp"

PipelineLayoutBuilder::PipelineLayoutBuilder(vk::Device& device)
{
    m_device = &device;
}

vk::PipelineLayout PipelineLayoutBuilder::build(
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::PipelineLayoutCreateInfo layoutInfo = {};

    layoutInfo.flags = vk::PipelineLayoutCreateFlagBits();
    layoutInfo.setLayoutCount = m_descriptorSetLayouts.size();
    layoutInfo.pSetLayouts = m_descriptorSetLayouts.data();

    vk::ResultValue<vk::PipelineLayout> layoutAttempt = 
        m_device->createPipelineLayout(layoutInfo);

    if(layoutAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create pipeline layout");
        return vk::PipelineLayout();
    }

    logger->print("Created pipeline layout");
    reset();

    vk::PipelineLayout handle = layoutAttempt.value;
    deletionQueue.push_back([logger, handle] (vk::Device device)->void{
        device.destroyPipelineLayout(handle);
        logger->print("Deleted pipeline layout");
    });

    return layoutAttempt.value;
}

void PipelineLayoutBuilder::add(vk::DescriptorSetLayout descriptorSetLayout) 
{
    m_descriptorSetLayouts.push_back(descriptorSetLayout);
}

void PipelineLayoutBuilder::reset()
{
    m_descriptorSetLayouts.clear();
}

std::vector<vk::ShaderEXT> make_shader_object(
    vk::Device logicalDevice, 
    const char* vertexFileName, 
    const char* fragmentFileName, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::ShaderCreateFlagsEXT flags = vk::ShaderCreateFlagBitsEXT::eLinkStage;
    vk::ShaderStageFlags nextStage = vk::ShaderStageFlagBits::eFragment;
    
    vk::ShaderCodeTypeEXT codeType = vk::ShaderCodeTypeEXT::eSpirv;
    const char* pName = "main";
    
    std::vector<char> vertexSrc = utils::read_file(vertexFileName);
    vk::ShaderCreateInfoEXT vertexInfo = vk::ShaderCreateInfoEXT(
        flags,
        vk::ShaderStageFlagBits::eVertex,
        nextStage,
        codeType,
        vertexSrc.size(),
        vertexSrc.data(),
        pName
    );

    nextStage = {};

    std::vector<char> fragmentSrc = utils::read_file(fragmentFileName);
    vk::ShaderCreateInfoEXT fragmentInfo = vk::ShaderCreateInfoEXT(
        flags,
        vk::ShaderStageFlagBits::eFragment,
        nextStage,
        codeType,
        fragmentSrc.size(),
        fragmentSrc.data(),
        pName
    );

    std::vector<vk::ShaderCreateInfoEXT> shaderInfo = {
        vertexInfo, fragmentInfo
    };

    vk::ResultValue<std::vector<vk::ShaderEXT>> createShadersAttempt =
        logicalDevice.createShadersEXT(shaderInfo);

    if(createShadersAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create shader object");
        return std::vector<vk::ShaderEXT>{};
    }

    std::vector<vk::ShaderEXT> shaders = createShadersAttempt.value;
    
    vk::ShaderEXT vertexShader = shaders[0];
    deletionQueue.push_back([logger, vertexShader] (vk::Device device)->void{
        device.destroyShaderEXT(vertexShader);
        logger->print("Deleted vertex shader");
    });
    
    vk::ShaderEXT fragmentShader = shaders[1];
    deletionQueue.push_back([logger, fragmentShader] (vk::Device device)->void{
        device.destroyShaderEXT(fragmentShader);
        logger->print("Deleted fragment shader");
    });

    return shaders;
}

#endif
