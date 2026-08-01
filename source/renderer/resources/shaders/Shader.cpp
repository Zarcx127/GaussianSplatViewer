#include "renderer/resources/shaders/Shader.hpp"

#include <vector>

#include "backend/Utils.hpp"

#include "logging/Logger.hpp"

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

namespace
{
    vk::ShaderModule make_shader_module(vk::Device device, const char* fileName)
    {
        Logger* logger = Logger::get_logger();

        std::vector<uint32_t> code = utils::read_spv_file(fileName);
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
