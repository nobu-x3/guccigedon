#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "core/types.h"

namespace render::vulkan::builder {

	VkCommandPoolCreateInfo command_pool_ci(uint32_t q_fam_index,
											VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo command_buffer_ai(
		VkCommandPool pool, uint32_t count = 1,
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkCommandBufferBeginInfo
	command_buffer_begin_info(VkCommandBufferUsageFlags flags = 0);

	VkFramebufferCreateInfo framebuffer_ci(VkRenderPass render_pass,
										   VkExtent2D extent);

	VkFenceCreateInfo fence_ci(VkFenceCreateFlags flags = 0);

	VkSemaphoreCreateInfo semaphore_ci(VkSemaphoreCreateFlags flags = 0);

	VkSubmitInfo submit_info(VkCommandBuffer* buf);

	VkPresentInfoKHR present_info();

	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass pass,
												VkExtent2D extent,
												VkFramebuffer framebuffer);

	VkImageCreateInfo image_ci(VkFormat format, VkImageUsageFlags usageFlags,
							   VkExtent3D extent, u32 mipLevels = 1,
							   u32 arrayLayers = 1);

	VkImageViewCreateInfo imageview_ci(VkFormat format, VkImage image,
									   VkImageAspectFlags aspectFlags,
									   u32 mipLevels = 1, u32 arrayLayers = 1);

	VkDescriptorSetLayoutBinding descriptorset_layout_binding(
		VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

	VkWriteDescriptorSet
	write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet,
							VkDescriptorBufferInfo* bufferInfo,
							uint32_t binding);

	VkSamplerCreateInfo
	sampler_create_info(VkFilter filters,
						VkSamplerAddressMode samplerAddressMode =
							VK_SAMPLER_ADDRESS_MODE_REPEAT, int mipLevels = 1);

	VkWriteDescriptorSet
	write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet,
						   VkDescriptorImageInfo* imageInfo, uint32_t binding);

	VkPipelineShaderStageCreateInfo
	pipeline_shader_stage_ci(VkShaderModule module,
							 VkShaderStageFlagBits flags);

	VkPipelineLayoutCreateInfo pipeline_layout_ci();

} // namespace render::vulkan::builder
