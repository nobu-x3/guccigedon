#include <algorithm>
#include <fstream>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/builders.h"

namespace render::vulkan::builder {

	VkCommandPoolCreateInfo
	command_pool_ci(uint32_t queueFamilyIndex,
					VkCommandPoolCreateFlags flags /*= 0*/) {
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}

	VkCommandBufferAllocateInfo command_buffer_ai(
		VkCommandPool pool, uint32_t count /*= 1*/,
		VkCommandBufferLevel level /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/) {
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.commandPool = pool;
		info.commandBufferCount = count;
		info.level = level;
		return info;
	}

	VkCommandBufferBeginInfo
	command_buffer_begin_info(VkCommandBufferUsageFlags flags /*= 0*/) {
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.pInheritanceInfo = nullptr;
		info.flags = flags;
		return info;
	}

	VkFramebufferCreateInfo framebuffer_ci(VkRenderPass renderPass,
										   VkExtent2D extent) {
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.pNext = nullptr;
		info.renderPass = renderPass;
		info.attachmentCount = 1;
		info.width = extent.width;
		info.height = extent.height;
		info.layers = 1;
		return info;
	}

	VkFenceCreateInfo fence_ci(VkFenceCreateFlags flags /*= 0*/) {
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}

	VkSemaphoreCreateInfo semaphore_ci(VkSemaphoreCreateFlags flags /*= 0*/) {
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = flags;
		return info;
	}

	VkSubmitInfo submit_info(VkCommandBuffer* buf) {
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;
		info.waitSemaphoreCount = 0;
		info.pWaitSemaphores = nullptr;
		info.pWaitDstStageMask = nullptr;
		info.commandBufferCount = 1;
		info.pCommandBuffers = buf;
		info.signalSemaphoreCount = 0;
		info.pSignalSemaphores = nullptr;
		return info;
	}

	VkPresentInfoKHR present_info() {
		VkPresentInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		info.pNext = nullptr;
		info.swapchainCount = 0;
		info.pSwapchains = nullptr;
		info.pWaitSemaphores = nullptr;
		info.waitSemaphoreCount = 0;
		info.pImageIndices = nullptr;

		return info;
	}

	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass renderPass,
												VkExtent2D windowExtent,
												VkFramebuffer framebuffer) {
		VkRenderPassBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;
		info.renderPass = renderPass;
		info.renderArea.offset.x = 0;
		info.renderArea.offset.y = 0;
		info.renderArea.extent = windowExtent;
		info.clearValueCount = 1;
		info.pClearValues = nullptr;
		info.framebuffer = framebuffer;
		return info;
	}

	VkImageCreateInfo image_ci(VkFormat format, VkImageUsageFlags usageFlags,
							   VkExtent3D extent) {
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.pNext = nullptr;

		info.imageType = VK_IMAGE_TYPE_2D;

		info.format = format;
		info.extent = extent;

		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = usageFlags;

		return info;
	}

	VkImageViewCreateInfo imageview_ci(VkFormat format, VkImage image,
									   VkImageAspectFlags aspectFlags) {
		// build a image-view for the depth image to use for rendering
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;

		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.image = image;
		info.format = format;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;
		info.subresourceRange.aspectMask = aspectFlags;

		return info;
	}

	VkDescriptorSetLayoutBinding
	descriptorset_layout_binding(VkDescriptorType type,
								 VkShaderStageFlags stageFlags,
								 uint32_t binding) {
		VkDescriptorSetLayoutBinding setbind = {};
		setbind.binding = binding;
		setbind.descriptorCount = 1;
		setbind.descriptorType = type;
		setbind.pImmutableSamplers = nullptr;
		setbind.stageFlags = stageFlags;
		return setbind;
	}

	VkWriteDescriptorSet
	write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet,
							VkDescriptorBufferInfo* bufferInfo,
							uint32_t binding) {
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstBinding = binding;
		write.dstSet = dstSet;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pBufferInfo = bufferInfo;
		return write;
	}

	VkSamplerCreateInfo
	sampler_create_info(VkFilter filters,
						VkSamplerAddressMode samplerAddressMode) {
		VkSamplerCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.pNext = nullptr;

		info.magFilter = filters;
		info.minFilter = filters;
		info.addressModeU = samplerAddressMode;
		info.addressModeV = samplerAddressMode;
		info.addressModeW = samplerAddressMode;

		return info;
	}

	VkWriteDescriptorSet
	write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet,
						   VkDescriptorImageInfo* imageInfo, uint32_t binding) {
		VkWriteDescriptorSet write = {};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;

		write.dstBinding = binding;
		write.dstSet = dstSet;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pImageInfo = imageInfo;

		return write;
	}

	VkPipelineShaderStageCreateInfo
	pipeline_shader_stage_ci(VkShaderModule module,
							 VkShaderStageFlagBits flags) {
		VkPipelineShaderStageCreateInfo ci{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr};
		ci.stage = flags;
		ci.module = module;
		ci.pName = "main";
		return ci;
	}

	VkPipelineLayoutCreateInfo pipeline_layout_ci() {
		VkPipelineLayoutCreateInfo info{
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr};
		info.flags = 0;
		info.setLayoutCount = 0;
		info.pSetLayouts = nullptr;
		info.pushConstantRangeCount = 0;
		info.pPushConstantRanges = nullptr;
		return info;
	}
} // namespace builder
