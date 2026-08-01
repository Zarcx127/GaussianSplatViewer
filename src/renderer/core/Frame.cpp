#include "renderer/core/Frame.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "factory/MeshFactory.hpp"

#include "renderer/core/Image.hpp"
#include "renderer/core/Synchronization.hpp"

#include "renderer/resources/Descriptors.hpp"

Frame::Frame(
    Swapchain* swapchain, 
    vk::Device device, 
    std::vector<vk::ShaderEXT>* shaders, 
    vk::CommandBuffer& commandBuffer,
    vk::DescriptorSetLayout* descriptorSetLayout,
    vk::DescriptorPool* descriptorPool,
    vk::PipelineLayout* pipelineLayout,
    Mesh* mesh,
    std::deque<std::function<void(vk::Device)>>& deletionQueue
) {
    this->swapchain = swapchain;
    this->shaders = shaders;
    this->commandBuffer = commandBuffer;

    m_descriptorSetLayout = descriptorSetLayout;
    m_descriptorPool = descriptorPool;
    m_pipelineLayout = pipelineLayout;
    
    m_descriptorSet = allocate_descriptor_set(device, *descriptorPool, *descriptorSetLayout);
    
    m_mesh = mesh;

    imageAquiredSemaphore = make_semaphore(device, deletionQueue);

    renderFinishedSemaphores.resize(swapchain->imageViews.size());
    for(vk::Semaphore& renderFinishedSemaphore : renderFinishedSemaphores)
        renderFinishedSemaphore = make_semaphore(device, deletionQueue);

    renderFinishedFence = make_fence(device, deletionQueue);
}

void Frame::record_command_buffer(uint32_t imageIndex, const glm::mat4& mvp) 
{
    (void) commandBuffer.reset();

    build_color_attachment(imageIndex);
    build_rendering_info();

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
    
    (void) commandBuffer.begin(beginInfo);

    transition_image_layout(
        commandBuffer,
        swapchain->images[imageIndex],
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eTopOfPipe,
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

    commandBuffer.pushConstants(
        *m_pipelineLayout,
        vk::ShaderStageFlagBits::eVertex,
        0,
        sizeof(glm::mat4),
        &mvp
    );
    
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
}

void Frame::build_color_attachment(uint32_t imageIndex)
{
    m_colorAttachment.imageView = swapchain->imageViews[imageIndex];
    m_colorAttachment.imageLayout = vk::ImageLayout::eAttachmentOptimal;
    m_colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    m_colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    m_colorAttachment.clearValue = vk::ClearValue({0.0f, 0.0f, 0.0f, 1.0f});
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

    commandBuffer.setColorBlendEnableEXT(0, VK_FALSE);
    commandBuffer.setColorBlendEquationEXT(0, equation);
	commandBuffer.setColorWriteMaskEXT(0, colorWriteMask);
	
    commandBuffer.setPolygonModeEXT(vk::PolygonMode::eFill);
	commandBuffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
	commandBuffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1, 1);
	commandBuffer.setCullMode(vk::CullModeFlagBits::eNone);
	commandBuffer.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    
    commandBuffer.setRasterizerDiscardEnable(VK_FALSE);
    commandBuffer.setAlphaToCoverageEnableEXT(VK_FALSE);
	commandBuffer.setDepthTestEnable(VK_FALSE);
	commandBuffer.setDepthWriteEnable(VK_FALSE);
	commandBuffer.setDepthBiasEnable(VK_FALSE);
	commandBuffer.setStencilTestEnable(VK_FALSE);
	commandBuffer.setPrimitiveRestartEnable(VK_FALSE);
}
