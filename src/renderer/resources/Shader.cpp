#include "renderer/resources/Shader.hpp"

#include <glm/glm.hpp>

#include <cstdint>
#include <array>

#include "logging/Logger.hpp"

#include "backend/Utils.hpp"

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
    layoutInfo.pushConstantRangeCount = m_pushConstantRanges.size();
    layoutInfo.pPushConstantRanges = m_pushConstantRanges.data();

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

void PipelineLayoutBuilder::add_push_constant_range(
    vk::ShaderStageFlags stageFlags, 
    uint32_t size
) {
    vk::PushConstantRange range = {};

    range.stageFlags = stageFlags;
    range.offset = 0;
    range.size = size;

    m_pushConstantRanges.push_back(range);
}

void PipelineLayoutBuilder::reset()
{
    m_descriptorSetLayouts.clear();
    m_pushConstantRanges.clear();
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

    std::array<vk::PushConstantRange, 1> pushRanges = {{
        vk::PushConstantRange{
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(glm::mat4)
        }
    }};
    
    std::vector<uint32_t> vertexSrc = utils::read_file(vertexFileName);
    vk::ShaderCreateInfoEXT vertexInfo = vk::ShaderCreateInfoEXT(
        flags,
        vk::ShaderStageFlagBits::eVertex,
        nextStage,
        codeType,
        vertexSrc.size() * sizeof(uint32_t),
        vertexSrc.data(),
        pName
    );

    vertexInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
    vertexInfo.pPushConstantRanges = pushRanges.data();

    nextStage = {};

    std::vector<uint32_t> fragmentSrc = utils::read_file(fragmentFileName);
    vk::ShaderCreateInfoEXT fragmentInfo = vk::ShaderCreateInfoEXT(
        flags,
        vk::ShaderStageFlagBits::eFragment,
        nextStage,
        codeType,
        fragmentSrc.size() * sizeof(uint32_t),
        fragmentSrc.data(),
        pName
    );

    fragmentInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
    fragmentInfo.pPushConstantRanges = pushRanges.data();

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

    logger->print("Created vertex shader");
    logger->print("Created fragment shader");

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

vk::ShaderEXT make_compute_shader(
    vk::Device logicalDevice, 
    const char* computeFileName,
    vk::DescriptorSetLayout* pLayouts,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::ShaderCodeTypeEXT codeType = vk::ShaderCodeTypeEXT::eSpirv;
    const char* pName = "main";
    
    std::vector<uint32_t> srcCode = utils::read_file(computeFileName);
    vk::ShaderCreateInfoEXT shaderInfo = {};

    shaderInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderInfo.codeType = codeType;
    shaderInfo.codeSize = srcCode.size() * sizeof(uint32_t);
    shaderInfo.pCode = srcCode.data();
    shaderInfo.pName = pName;
    shaderInfo.setLayoutCount = 1;
    shaderInfo.pSetLayouts = pLayouts;

    vk::ResultValue<vk::ShaderEXT> createShadersAttempt = 
        logicalDevice.createShaderEXT(shaderInfo);

    if(createShadersAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create compute shader object");
        return vk::ShaderEXT();
    }

    vk::ShaderEXT computeShader = createShadersAttempt.value;
    
    deletionQueue.push_back([logger, computeShader] (vk::Device device)->void{
        device.destroyShaderEXT(computeShader);
        logger->print("Deleted compute shader");
    });

    return computeShader;
}
