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

#include "renderer/LoadingScreen.hpp"

#include "logging/Logger.hpp"

#include "renderer/core/Command.hpp"
#include "renderer/core/Image.hpp"
#include "renderer/core/Synchronization.hpp"

#include "renderer/resources/shaders/Shader.hpp"

#include "renderer/FrameCommands.hpp"
#include "renderer/ShaderPaths.hpp"

bool LoadingScreen::build(
    VulkanContext& vulkanContext,
    RenderResources& renderResources,
    vk::PipelineLayout pipelineLayout
) {
    if(
        !pipelineLayout ||
        (renderResources.color_image_count() == 0)
    ) {
        return false;
    }

    m_vulkanContext = &vulkanContext;
    m_renderResources = &renderResources;
    m_pipelineLayout = pipelineLayout;

    vk::Device device = m_vulkanContext->logical_device();

    m_commandBuffer = make_command_buffer(
        device,
        m_vulkanContext->command_pool(),
        m_deletionQueue
    );

    if(!m_commandBuffer)
    {
        destroy();
        return false;
    }

    m_imageAcquiredSemaphore = make_semaphore(
        device,
        m_deletionQueue
    );

    if(!m_imageAcquiredSemaphore)
    {
        destroy();
        return false;
    }

    m_renderFinishedSemaphores.resize(
        m_renderResources->color_image_count()
    );

    for(vk::Semaphore& semaphore : m_renderFinishedSemaphores)
    {
        semaphore = make_semaphore(
            device,
            m_deletionQueue
        );

        if(!semaphore)
        {
            destroy();
            return false;
        }
    }

    m_renderFinishedFence = make_fence(
        device,
        m_deletionQueue
    );

    if(!m_renderFinishedFence)
    {
        destroy();
        return false;
    }

    if(!build_pipeline())
    {
        destroy();
        return false;
    }

    return true;
}

bool LoadingScreen::draw(float time)
{
    Logger* logger = Logger::get_logger();

    if(
        !m_vulkanContext ||
        !m_renderResources ||
        !m_pipeline ||
        !m_pipelineLayout ||
        !m_commandBuffer ||
        !m_imageAcquiredSemaphore ||
        !m_renderFinishedFence ||
        (
            m_renderFinishedSemaphores.size() !=
            m_renderResources->color_image_count()
        )
    ) {
        return false;
    }

    vk::Device device = m_vulkanContext->logical_device();
    vk::Queue graphicsQueue = m_vulkanContext->graphics_queue();

    if(
        device.waitForFences(
            m_renderFinishedFence,
            vk::True,
            UINT64_MAX
        ) != vk::Result::eSuccess
    ) {
        logger->print("Failed to wait for loading screen fence");
        return false;
    }

    uint32_t imageIndex = 0;

    VkResult acquireResult = vkAcquireNextImageKHR(
        static_cast<VkDevice>(device),
        static_cast<VkSwapchainKHR>(
            m_renderResources->swapchain_handle()
        ),
        UINT64_MAX,
        static_cast<VkSemaphore>(m_imageAcquiredSemaphore),
        VK_NULL_HANDLE,
        &imageIndex
    );

    if(
        (acquireResult != VK_SUCCESS) &&
        (acquireResult != VK_SUBOPTIMAL_KHR)
    ) {
        logger->print("Failed to acquire loading screen swapchain image");
        return false;
    }

    if(imageIndex >= m_renderFinishedSemaphores.size())
        return false;

    if(
        device.resetFences(
            m_renderFinishedFence
        ) != vk::Result::eSuccess
    ) {
        logger->print("Failed to reset loading screen fence");
        return false;
    }

    if(!record(imageIndex, time))
        return false;

    vk::PipelineStageFlags waitStage =
        vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBufferCount = 1;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.signalSemaphoreCount = 1;

    submitInfo.pCommandBuffers = &m_commandBuffer;
    submitInfo.pWaitSemaphores = &m_imageAcquiredSemaphore;
    submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[imageIndex];
    submitInfo.pWaitDstStageMask = &waitStage;

    if(
        graphicsQueue.submit(
            submitInfo,
            m_renderFinishedFence
        ) != vk::Result::eSuccess
    ) {
        logger->print("Failed to submit loading screen command buffer");
        return false;
    }

    VkSwapchainKHR rawSwapchain = static_cast<VkSwapchainKHR>(
        m_renderResources->swapchain_handle()
    );

    VkSemaphore rawWaitSemaphore = static_cast<VkSemaphore>(
        m_renderFinishedSemaphores[imageIndex]
    );

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &rawWaitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &rawSwapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    VkResult presentResult = vkQueuePresentKHR(
        static_cast<VkQueue>(graphicsQueue),
        &presentInfo
    );

    if(
        (presentResult != VK_SUCCESS) &&
        (presentResult != VK_SUBOPTIMAL_KHR)
    ) {
        logger->print("Failed to present loading screen");
        return false;
    }

    return true;
}

void LoadingScreen::destroy()
{
    if(!m_vulkanContext)
        return;

    vk::Queue graphicsQueue = m_vulkanContext->graphics_queue();

    if(graphicsQueue)
        (void) graphicsQueue.waitIdle();

    vk::Device device = m_vulkanContext->logical_device();

    while(!m_deletionQueue.empty())
    {
        if(device)
            (m_deletionQueue.back())(device);

        m_deletionQueue.pop_back();
    }

    m_pipelineLayout = vk::PipelineLayout();
    m_pipeline = vk::Pipeline();

    m_commandBuffer = vk::CommandBuffer();

    m_imageAcquiredSemaphore = vk::Semaphore();
    m_renderFinishedSemaphores.clear();
    m_renderFinishedFence = vk::Fence();

    m_renderResources = nullptr;
    m_vulkanContext = nullptr;
}

LoadingScreen::~LoadingScreen()
{
    destroy();
}

bool LoadingScreen::build_pipeline()
{
    m_pipeline = make_graphics_pipeline(
        m_vulkanContext->logical_device(),
        shader::loading::loadingScreenVertex,
        shader::loading::loadingScreenFragment,
        m_pipelineLayout,
        m_renderResources->color_format(),
        m_deletionQueue
    );

    return static_cast<bool>(m_pipeline);
}

bool LoadingScreen::record(uint32_t imageIndex, float time)
{
    Logger* logger = Logger::get_logger();

    if(imageIndex >= m_renderResources->color_image_count())
        return false;

    if(m_commandBuffer.reset() != vk::Result::eSuccess)
    {
        logger->print("Failed to reset loading screen command buffer");
        return false;
    }

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    if(m_commandBuffer.begin(beginInfo) != vk::Result::eSuccess)
    {
        logger->print("Failed to begin loading screen command buffer");
        return false;
    }

    vk::Image colorImage =
        m_renderResources->color_image(imageIndex);

    transition_image_layout(
        m_commandBuffer,
        colorImage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::AccessFlagBits::eNone,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::ImageAspectFlagBits::eColor
    );

    vk::RenderingAttachmentInfoKHR colorAttachment = {};
    colorAttachment.imageView =
        m_renderResources->color_image_view(imageIndex);

    colorAttachment.imageLayout =
        vk::ImageLayout::eColorAttachmentOptimal;

    colorAttachment.loadOp =
        vk::AttachmentLoadOp::eDontCare;

    colorAttachment.storeOp =
        vk::AttachmentStoreOp::eStore;

    vk::Extent2D extent = m_renderResources->extent();

    vk::RenderingInfoKHR renderingInfo = {};
    renderingInfo.renderArea.extent = extent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vk::Viewport viewport = {};
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.extent = extent;

    LoadingScreenPushConstant pushConstants = {};
    pushConstants.width = static_cast<float>(extent.width);
    pushConstants.height = static_cast<float>(extent.height);
    pushConstants.time = time;

    m_commandBuffer.beginRenderingKHR(renderingInfo);

    m_commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        m_pipeline
    );

    m_commandBuffer.setViewport(
        0,
        1,
        &viewport
    );

    m_commandBuffer.setScissor(
        0,
        1,
        &scissor
    );

    m_commandBuffer.pushConstants(
        m_pipelineLayout,
        (
            vk::ShaderStageFlagBits::eCompute |
            vk::ShaderStageFlagBits::eFragment
        ),
        sizeof(FramePushConstant),
        sizeof(LoadingScreenPushConstant),
        &pushConstants
    );

    m_commandBuffer.draw(3, 1, 0, 0);

    m_commandBuffer.endRenderingKHR();

    transition_image_layout(
        m_commandBuffer,
        colorImage,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits::eColorAttachmentWrite,
        vk::AccessFlagBits::eNone,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eBottomOfPipe,
        vk::ImageAspectFlagBits::eColor
    );

    if(m_commandBuffer.end() != vk::Result::eSuccess)
    {
        logger->print("Failed to end loading screen command buffer");
        return false;
    }

    return true;
}
