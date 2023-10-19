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
		std::vector<VkPipelineShaderStageCreateInfo> _shader_stages{};
		array_list<VkVertexInputBindingDescription> _vertex_bindings{};
		array_list<VkVertexInputAttributeDescription> _vertex_attributes{};

		VkPipelineInputAssemblyStateCreateInfo _input_assembly{
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false};

		VkViewport _viewport = {};
		VkRect2D _scissor = {};

		VkPipelineRasterizationStateCreateInfo _rasterizer{
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			nullptr,
			0,
			false,
			false,
			VK_POLYGON_MODE_FILL,
			0,
			VK_FRONT_FACE_CLOCKWISE,
			false,
			0.f,
			0.f,
			0.f,
			0.f};

		VkPipelineColorBlendAttachmentState _color_blend_attachment{
			false,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_OP_ADD};

		VkPipelineMultisampleStateCreateInfo _multisampling{
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_SAMPLE_COUNT_1_BIT,
			false,
			1.f,
			nullptr,
			0,
			0};

		VkPipelineDepthStencilStateCreateInfo _depth_stencil{
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			nullptr,
			0,
			false,
			false,
			VK_COMPARE_OP_LESS_OR_EQUAL,
			false,
			false,
			{},
			{},
			0.f,
			1.f};

		VkPipelineColorBlendStateCreateInfo _color_blend_state{
			// @TODO: add color attachments array_list
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			nullptr,
			0,
			false,
			VK_LOGIC_OP_COPY,
			1,
			&_color_blend_attachment,
			{0.f, 0.f, 0.f, 0.f}};

		array_list<VkDynamicState> _dynamic_states{};

		VkPipelineDynamicStateCreateInfo _dynamic_state_cis{
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 0,
			nullptr};

		VkPipelineLayout _pipeline_layout{};

	public:
		PipelineBuilder &add_shader_stage(VkPipelineShaderStageCreateInfo ci);

		PipelineBuilder &add_vertex_binding(u32 stride,
											VkVertexInputRate input_rate);

		PipelineBuilder &add_vertex_attribute(u32 binding, u32 location,
											  VkFormat format, u32 offset);

		PipelineBuilder &
		set_input_assembly(VkPrimitiveTopology topology,
						   VkPipelineInputAssemblyStateCreateFlags create_flags,
						   bool primitive_restart_enable);

		PipelineBuilder &set_cull_mode(VkCullModeFlags cull_mode,
									   VkFrontFace front_face);

		PipelineBuilder &set_polygon_mode(VkPolygonMode mode);

		PipelineBuilder &set_default_color_blend_enabled(bool val);

		PipelineBuilder &set_multisampling_enabled(
			bool val, VkSampleCountFlagBits count = VK_SAMPLE_COUNT_1_BIT);

		PipelineBuilder &
		set_depth_testing(bool testEnabled, bool depthWrite,
						  VkCompareOp compare_op = VK_COMPARE_OP_LESS_OR_EQUAL);

		PipelineBuilder &
		set_color_blending_enabled(bool enabled,
								   VkLogicOp op = VK_LOGIC_OP_COPY);

		PipelineBuilder &add_dynamic_state(VkDynamicState dynamic_state);

		PipelineBuilder &set_viewport(VkViewport viewport);

		PipelineBuilder &set_scissor(VkRect2D scissor);

		VkPipelineLayout build_layout();

		VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
	};
} // namespace vkbuild
