#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include "../core/core.h"
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

	class PipelineBuilder {
	private:
		std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;
		VkPipelineVertexInputStateCreateInfo _vertex_input = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr};
		VkPipelineInputAssemblyStateCreateInfo _input_assembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr};
		VkViewport _viewport = {};
		VkRect2D _scissor = {};
		VkPipelineRasterizationStateCreateInfo _rasterizer = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr};
		VkPipelineColorBlendAttachmentState _color_blend_attachment = {};
		VkPipelineMultisampleStateCreateInfo _multisampling = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr};
		VkPipelineLayout _pipeline_layout {};

	public:
		PipelineBuilder &add_shader_stage(VkPipelineShaderStageCreateInfo ci);
		

		PipelineBuilder &
		set_vertex_input(VkPipelineVertexInputStateCreateInfo ci);

		PipelineBuilder &
		set_input_assembly(VkPipelineInputAssemblyStateCreateInfo ci);

		PipelineBuilder &set_viewport(VkViewport viewport);

		PipelineBuilder &set_scissor(VkRect2D scissor);

		PipelineBuilder &
		set_rasterizer(VkPipelineRasterizationStateCreateInfo ci);

		PipelineBuilder &
		set_color_blend(VkPipelineColorBlendAttachmentState state);

		PipelineBuilder &
		set_multisampling(VkPipelineMultisampleStateCreateInfo ci);

		VkPipelineLayout build_layout();

		VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
	};
} // namespace vkbuild
