#include "renderer/core/Frame.hpp"

#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "factory/MeshFactory.hpp"

#include "renderer/core/Image.hpp"
#include "renderer/core/Synchronization.hpp"

Frame::Frame(
    Swapchain* swapchain, 
    vk::Device device, 
    std::vector<vk::ShaderEXT>* shaders, 
    const vk::ShaderEXT* computeShader,
    vk::CommandBuffer& commandBuffer,
    const std::vector<vk::DescriptorSet>* swapchainImageDescriptorSets,
    const vk::PipelineLayout* pipelineLayout,
    const AllocatedImage* depthImage,
    Mesh* mesh, 
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    this->swapchain = swapchain;
    this->shaders = shaders;
    this->commandBuffer = commandBuffer;

    m_computeShader = computeShader;
    m_swapchainImageDescriptorSets = swapchainImageDescriptorSets;
    m_pipelineLayout = pipelineLayout;
    m_depthImage = depthImage;
        
    m_mesh = mesh;

    imageAcquiredSemaphore = make_semaphore(device, deletionQueue);

    renderFinishedSemaphores.resize(swapchain->imageViews.size());
    for(vk::Semaphore& renderFinishedSemaphore : renderFinishedSemaphores)
        renderFinishedSemaphore = make_semaphore(device, deletionQueue);

    renderFinishedFence = make_fence(device, deletionQueue);
}

void Frame::record_command_buffer(
    uint32_t imageIndex, const FramePushConstant& pushConstants
) {
    (void) commandBuffer.reset();

    build_color_attachment(imageIndex);
    build_depth_attachment();
    build_rendering_info();

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
    
    (void) commandBuffer.begin(beginInfo);

    transition_image_layout(
        commandBuffer,
        swapchain->images[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eGeneral,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eShaderWrite,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eComputeShader 
    );

    commandBuffer.pushConstants(
        *m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eCompute,
        0,
        sizeof(FramePushConstant),
        &pushConstants
    );

    record_compute_background(imageIndex);

    transition_image_layout(
        commandBuffer,
        swapchain->images[imageIndex],
        vk::ImageLayout::eGeneral,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits::eShaderWrite,
        vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eComputeShader,
        vk::PipelineStageFlagBits::eColorAttachmentOutput 
    );

    initialize_render_state();

    commandBuffer.beginRenderingKHR(m_renderingInfo);

    vk::ShaderStageFlagBits stages[2] = {
        vk::ShaderStageFlagBits::eVertex,
        vk::ShaderStageFlagBits::eFragment
    };

    commandBuffer.bindShadersEXT(stages, *shaders);
    commandBuffer.bindVertexBuffers(0, 1, &(m_mesh->buffer), &(m_mesh->offset));
    
    commandBuffer.draw(m_mesh->numOfVertices, 1, 0, 0);

    commandBuffer.endRenderingKHR();

    transition_image_layout(
        commandBuffer,
        swapchain->images[imageIndex],
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eNone,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe
    );

    (void) commandBuffer.end();
}

void Frame::build_rendering_info()
{
    m_renderingInfo.flags = vk::RenderingFlagsKHR();
    m_renderingInfo.renderArea = vk::Rect2D({0, 0}, swapchain->extent);
    m_renderingInfo.viewMask = 0;
    m_renderingInfo.layerCount = 1;
    m_renderingInfo.colorAttachmentCount = 1;
    m_renderingInfo.pColorAttachments = &m_colorAttachment;
    m_renderingInfo.pDepthAttachment = &m_depthAttachment;
    m_renderingInfo.pStencilAttachment = nullptr;
}

void Frame::build_color_attachment(uint32_t imageIndex)
{
    m_colorAttachment.imageView = swapchain->imageViews[imageIndex];
    m_colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    m_colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;
    m_colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    m_colorAttachment.clearValue = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
}

void Frame::build_depth_attachment()
{
    vk::ClearValue clearValue = {};
    clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    m_depthAttachment.imageView = m_depthImage->imageView;
    m_depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
    m_depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    m_depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
    m_depthAttachment.clearValue = clearValue;
}

void Frame::initialize_render_state()
{
    vk::VertexInputBindingDescription2EXT binding = get_binding_description();
    std::vector<vk::VertexInputAttributeDescription2EXT> attributes = 
        get_attribute_descriptions();

    commandBuffer.setVertexInputEXT(1, &binding, 2, attributes.data());

    vk::Viewport viewport = vk::Viewport(
        0.0f, 0.0f, swapchain->extent.width, swapchain->extent.height, 0.0f, 1.0f
    );

    vk::Rect2D scissor = vk::Rect2D({0, 0}, swapchain->extent);

    vk::ColorBlendEquationEXT equation;
	
    equation.colorBlendOp = vk::BlendOp::eAdd;
	equation.dstColorBlendFactor = vk::BlendFactor::eZero;
	equation.srcColorBlendFactor = vk::BlendFactor::eOne;

    using Flag = vk::ColorComponentFlagBits;
	vk::ColorComponentFlags colorWriteMask = 
        (Flag::eR | Flag::eG | Flag::eB | Flag::eA);
    
    commandBuffer.setViewportWithCount(viewport);
    commandBuffer.setScissorWithCount(scissor);

    commandBuffer.setColorBlendEnableEXT(0, vk::False);
    commandBuffer.setColorBlendEquationEXT(0, equation);
	commandBuffer.setColorWriteMaskEXT(0, colorWriteMask);
	
    commandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill);
	commandBuffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
	commandBuffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1, 1);
	commandBuffer.setCullMode(vk::CullModeFlagBits::eNone);
	commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    
    commandBuffer.setRasterizerDiscardEnable(vk::False);
    commandBuffer.setAlphaToCoverageEnableEXT(vk::False);
	commandBuffer.setDepthTestEnable(vk::True);
	commandBuffer.setDepthWriteEnable(vk::True);
	commandBuffer.setDepthBiasEnable(vk::False);
	commandBuffer.setStencilTestEnable(vk::False);
	commandBuffer.setPrimitiveRestartEnable(vk::False);

    commandBuffer.setDepthCompareOp(vk::CompareOp::eLess);
}

void Frame::record_compute_background(uint32_t imageIndex)
{
    std::array<vk::ShaderStageFlagBits, 1> stages = {
        vk::ShaderStageFlagBits::eCompute
    };

    std::array<vk::ShaderEXT, 1> computeShaders = {
        *m_computeShader
    };

    commandBuffer.bindShadersEXT(stages, computeShaders);

    vk::DescriptorSet descriptorSet = (*m_swapchainImageDescriptorSets)[imageIndex];
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        *m_pipelineLayout,
        0,
        1,
        &descriptorSet,
        0,
        nullptr
    );

    uint32_t pixelCount = swapchain->extent.width * swapchain->extent.height;
    uint32_t groupCount = (pixelCount + 63) / 64;

    commandBuffer.dispatch(groupCount, 1, 1);
}
