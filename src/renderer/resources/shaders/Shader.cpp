#include "renderer/resources/shaders/Shader.hpp"

#include <cstdint>
#include <array>

#include "logging/Logger.hpp"

#include "backend/Utils.hpp"

std::vector<vk::ShaderEXT> make_shader_object(
    vk::Device device, 
    const char* vertexFileName, 
    const char* fragmentFileName, 
    const ShaderInterface& shaderInterface,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    const std::vector<vk::DescriptorSetLayout>& setLayouts =
        shaderInterface.get_descriptor_set_layouts();
    
    const std::vector<vk::PushConstantRange>& pushRanges =
        shaderInterface.get_push_constant_ranges();

    vk::ShaderCreateFlagsEXT flags = vk::ShaderCreateFlagBitsEXT::eLinkStage;
    vk::ShaderStageFlags nextStage = vk::ShaderStageFlagBits::eFragment;
    
    vk::ShaderCodeTypeEXT codeType = vk::ShaderCodeTypeEXT::eSpirv;
    const char* pName = "main";

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

    vertexInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    vertexInfo.pSetLayouts = setLayouts.data();
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

    fragmentInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    fragmentInfo.pSetLayouts = setLayouts.data();
    fragmentInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
    fragmentInfo.pPushConstantRanges = pushRanges.data();

    std::vector<vk::ShaderCreateInfoEXT> shaderInfo = {
        vertexInfo, fragmentInfo
    };

    vk::ResultValue<std::vector<vk::ShaderEXT>> createShadersAttempt =
        device.createShadersEXT(shaderInfo);

    if(createShadersAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create shader object");
        return std::vector<vk::ShaderEXT>{};
    }

    logger->print("Created vertex shader");
    logger->print("Created fragment shader");

    std::vector<vk::ShaderEXT> shaders = createShadersAttempt.value;
    
    vk::ShaderEXT vertexShader = shaders[0];
    deletionQueue.push_back(
        [logger, vertexShader] (vk::Device device)->void {
            device.destroyShaderEXT(vertexShader);
            logger->print("Deleted vertex shader");
        }
    );
    
    vk::ShaderEXT fragmentShader = shaders[1];
    deletionQueue.push_back(
        [logger, fragmentShader] (vk::Device device)->void {
            device.destroyShaderEXT(fragmentShader);
            logger->print("Deleted fragment shader");
        }
    );

    return shaders;
}

vk::ShaderEXT make_compute_shader(
    vk::Device device, 
    const char* computeFileName, 
    const ShaderInterface& shaderInterface,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    const std::vector<vk::DescriptorSetLayout>& setLayouts =
        shaderInterface.get_descriptor_set_layouts();
    
    const std::vector<vk::PushConstantRange>& pushRanges =
        shaderInterface.get_push_constant_ranges();

    vk::ShaderCodeTypeEXT codeType = vk::ShaderCodeTypeEXT::eSpirv;
    const char* pName = "main";
    
    std::vector<uint32_t> srcCode = utils::read_file(computeFileName);
    vk::ShaderCreateInfoEXT shaderInfo = {};

    shaderInfo.stage = vk::ShaderStageFlagBits::eCompute;
    shaderInfo.codeType = codeType;
    shaderInfo.codeSize = srcCode.size() * sizeof(uint32_t);
    shaderInfo.pCode = srcCode.data();
    shaderInfo.pName = pName;
    shaderInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    shaderInfo.pSetLayouts = setLayouts.data();
    shaderInfo.pushConstantRangeCount = static_cast<uint32_t>(pushRanges.size());
    shaderInfo.pPushConstantRanges = pushRanges.data();

    vk::ResultValue<vk::ShaderEXT> createShadersAttempt = 
        device.createShaderEXT(shaderInfo);
    
    if(createShadersAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create compute shader object");
        return vk::ShaderEXT();
    }

    vk::ShaderEXT computeShader = createShadersAttempt.value;
    
    deletionQueue.push_back(
        [logger, computeShader] (vk::Device device)->void {
            device.destroyShaderEXT(computeShader);
            logger->print("Deleted compute shader");
        }
    );

    return computeShader;
}
