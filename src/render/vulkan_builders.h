#pragma once

#include <cstdint>
#include "vulkan_types.h"

namespace vkbuild {
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

	VkSubmitInfo submit_info(VkCommandBuffer *buf);

	VkPresentInfoKHR present_info();

	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass pass,
												VkExtent2D extent,
												VkFramebuffer framebuffer);
} // namespace vkbuild
