/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#include "renderer/resources/shaders/Shader.hpp"

#ifdef DEBUG

#include <vector>

#include "backend/Utils.hpp"

#endif

#include "logging/Logger.hpp"

namespace
{
    vk::ShaderModule make_shader_module(vk::Device device, ShaderAsset shader);
}

vk::Pipeline make_graphics_pipeline(
    vk::Device device,
    ShaderAsset vertexShader,
    ShaderAsset fragmentShader,
    vk::PipelineLayout pipelineLayout,
    vk::Format colorFormat,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::ShaderModule vertexModule = make_shader_module(
        device,
        vertexShader
    );

    if(!vertexModule)
        return vk::Pipeline();

    vk::ShaderModule fragmentModule = make_shader_module(
        device,
        fragmentShader
    );

    if(!fragmentModule)
    {
        device.destroyShaderModule(vertexModule);
        return vk::Pipeline();
    }

    vk::PipelineShaderStageCreateInfo stageInfo[2] = {};

    stageInfo[0].stage = vk::ShaderStageFlagBits::eVertex;
    stageInfo[0].module = vertexModule;
    stageInfo[0].pName = "main";

    stageInfo[1].stage = vk::ShaderStageFlagBits::eFragment;
    stageInfo[1].module = fragmentModule;
    stageInfo[1].pName = "main";

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;

    vk::PipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.viewportCount = 1;
    viewportInfo.scissorCount = 1;

    vk::PipelineRasterizationStateCreateInfo rasterizationInfo = {};
    rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
    rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
    rasterizationInfo.lineWidth = 1.0f;

    vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

    vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = (
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    );

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;

    vk::DynamicState dynamicStates[] = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.dynamicStateCount = 2;
    dynamicStateInfo.pDynamicStates = dynamicStates;

    vk::PipelineRenderingCreateInfoKHR renderingInfo = {};
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    vk::GraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.pNext = &renderingInfo;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stageInfo;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;
    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = pipelineLayout;

    vk::ResultValue<vk::Pipeline> pipelineAttempt =
        device.createGraphicsPipeline(
            vk::PipelineCache(),
            pipelineInfo
        );

    device.destroyShaderModule(vertexModule);
    device.destroyShaderModule(fragmentModule);

    if(pipelineAttempt.result != vk::Result::eSuccess)
    {
        logger->print("Failed to create graphics pipeline");
        return vk::Pipeline();
    }

    vk::Pipeline pipeline = pipelineAttempt.value;
    deletionQueue.push_back(
        [logger, pipeline] (vk::Device device)->void {
            device.destroyPipeline(pipeline);
            logger->print("Deleted graphics pipeline");
        }
    );

    logger->print("Created graphics pipeline");

    return pipeline;
}

vk::Pipeline make_compute_pipeline(
    vk::Device device,
    ShaderAsset computeShader,
    vk::PipelineLayout pipelineLayout,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    Logger* logger = Logger::get_logger();

    vk::ShaderModule computeModule = make_shader_module(device, computeShader);
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

namespace
{
    vk::ShaderModule make_shader_module(vk::Device device, ShaderAsset shader)
    {
        Logger* logger = Logger::get_logger();

        vk::ShaderModuleCreateInfo moduleInfo = {};

#ifdef DEBUG
        std::vector<uint32_t> code = utils::read_spv_file(shader);

        if(code.empty())
        {
            logger->print("Shader file is empty or failed to load");
            return vk::ShaderModule();
        }

        moduleInfo.codeSize = code.size() * sizeof(uint32_t);
        moduleInfo.pCode = code.data();
#else
        if(!shader.data || (shader.size == 0))
        {
            logger->print("Shader asset is empty");
            return vk::ShaderModule();
        }

        moduleInfo.codeSize = shader.size;
        moduleInfo.pCode = shader.data;
#endif

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
