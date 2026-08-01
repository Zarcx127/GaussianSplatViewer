#include "renderer/resources/shaders/Shader.hpp"

#include <array>
#include <vector>

#include "logging/Logger.hpp"

#include "backend/Utils.hpp"

namespace
{
    vk::ShaderModule make_shader_module(vk::Device device, const char* fileName);
}

vk::Pipeline make_compute_pipeline(
    vk::Device device,
    const char* computeFileName,
    vk::PipelineLayout pipelineLayout,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::ShaderModule computeModule = make_shader_module(device, computeFileName);
    if(!computeModule)
        return vk::Pipeline();

    vk::PipelineShaderStageCreateInfo stageInfo = {};
    stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
    stageInfo.module = computeModule;
    stageInfo.pName = "main";

    vk::ComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.stage = stageInfo;
    pipelineInfo.layout = pipelineLayout;

    vk::ResultValue<vk::Pipeline> pipelineAttempt =
        device.createComputePipeline(vk::PipelineCache(), pipelineInfo);

    device.destroyShaderModule(computeModule);

    if(pipelineAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create compute pipeline");
        return vk::Pipeline();
    }

    vk::Pipeline pipeline = pipelineAttempt.value;

    deletionQueue.push_back(
        [logger, pipeline] (vk::Device device)->void {
            device.destroyPipeline(pipeline);
            logger->print("Deleted compute pipeline");
        }
    );

    logger->print("Created compute pipeline");
    return pipeline;
}

vk::Pipeline make_graphics_pipeline(
    vk::Device device,
    const char* vertexFileName,
    const char* fragmentFileName,
    const GraphicsPipelineConfig& config,
    vk::PipelineLayout pipelineLayout,
    vk::Format colorFormat,
    vk::Format depthFormat,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::ShaderModule vertexModule = make_shader_module(device, vertexFileName);
    if(!vertexModule)
        return vk::Pipeline();

    vk::ShaderModule fragmentModule = make_shader_module(device, fragmentFileName);
    if(!fragmentModule)
    {
        device.destroyShaderModule(vertexModule);
        return vk::Pipeline();
    }

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {};

    shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
    shaderStages[0].module = vertexModule;
    shaderStages[0].pName = "main";

    shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
    shaderStages[1].module = fragmentModule;
    shaderStages[1].pName = "main";

    bool hasVertexBindings = !config.vertexBindingDescriptions.empty();
    bool hasVertexAttributes = !config.vertexAttributeDescriptions.empty();

    if(hasVertexBindings != hasVertexAttributes)
    {
        device.destroyShaderModule(fragmentModule);
        device.destroyShaderModule(vertexModule);

        logger->print("Invalid graphics pipeline vertex input configuration");
        return vk::Pipeline();
    }

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
    
    vertexInputInfo.vertexBindingDescriptionCount = 
        static_cast<uint32_t>(config.vertexBindingDescriptions.size());
    
    vertexInputInfo.pVertexBindingDescriptions = (
        config.vertexBindingDescriptions.empty()
            ? nullptr
            : config.vertexBindingDescriptions.data()
    );

    vertexInputInfo.vertexAttributeDescriptionCount = 
        static_cast<uint32_t>(config.vertexAttributeDescriptions.size());
    
    vertexInputInfo.pVertexAttributeDescriptions = (
        config.vertexAttributeDescriptions.empty()
            ? nullptr
            : config.vertexAttributeDescriptions.data()
    );

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.topology = config.topology;
    inputAssemblyInfo.primitiveRestartEnable = vk::False;

    vk::PipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.depthClampEnable = vk::False;
    rasterizationInfo.rasterizerDiscardEnable = vk::False;
    rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizationInfo.cullMode = config.cullMode;
    rasterizationInfo.frontFace = config.frontFace;
    rasterizationInfo.depthBiasEnable = vk::False;
    rasterizationInfo.lineWidth = 1.0f;

    vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampleInfo.sampleShadingEnable = vk::False;

    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo = {};
    depthStencilInfo.depthTestEnable = (config.depthTest ? vk::True : vk::False);
    depthStencilInfo.depthWriteEnable = (config.depthWrite ? vk::True : vk::False);
    depthStencilInfo.depthCompareOp = config.depthCompareOp;
    depthStencilInfo.depthBoundsTestEnable = vk::False;
    depthStencilInfo.stencilTestEnable = vk::False;

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.logicOpEnable = vk::False;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &config.colorBlendAttachment;

    std::array<vk::DynamicState, 2> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateInfo.pDynamicStates = dynamicStates.data();

    vk::PipelineRenderingCreateInfoKHR renderingInfo = {};
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;
    renderingInfo.depthAttachmentFormat = depthFormat;

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = vk::RenderPass();
    pipelineInfo.subpass = 0;

    vk::ResultValue<vk::Pipeline> pipelineAttempt =
        device.createGraphicsPipeline(vk::PipelineCache(), pipelineInfo);

    device.destroyShaderModule(fragmentModule);
    device.destroyShaderModule(vertexModule);

    if(pipelineAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create graphics pipeline");
        return vk::Pipeline();
    }

    vk::Pipeline pipeline = pipelineAttempt.value;

    deletionQueue.push_back(
        [logger, pipeline](vk::Device device)->void {
            device.destroyPipeline(pipeline);
            logger->print("Deleted graphics pipeline");
        }
    );

    logger->print("Created graphics pipeline");
    return pipeline;
}

namespace
{
    vk::ShaderModule make_shader_module(vk::Device device, const char* fileName)
    {
        Logger* logger = Logger::get_logger();

        std::vector<uint32_t> code = utils::read_file(fileName);
        if(code.empty())
        {
            logger->print("Shader file is empty or failed to load");
            return vk::ShaderModule();
        }

        vk::ShaderModuleCreateInfo moduleInfo = {};
        moduleInfo.codeSize = code.size() * sizeof(uint32_t);
        moduleInfo.pCode = code.data();

        vk::ResultValue<vk::ShaderModule> moduleAttempt =
            device.createShaderModule(moduleInfo);

        if(moduleAttempt.result != vk::Result::eSuccess)
        {
            logger->print("Failed to create shader module");
            return vk::ShaderModule();
        }

        return moduleAttempt.value;
    }
}