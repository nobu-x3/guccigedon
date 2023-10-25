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

	VkSubmitInfo submit_info(VkCommandBuffer* buf);

	VkPresentInfoKHR present_info();

	VkRenderPassBeginInfo renderpass_begin_info(VkRenderPass pass,
												VkExtent2D extent,
												VkFramebuffer framebuffer);

	VkImageCreateInfo image_ci(VkFormat format, VkImageUsageFlags usageFlags,
							   VkExtent3D extent);

	VkImageViewCreateInfo imageview_ci(VkFormat format, VkImage image,
									   VkImageAspectFlags aspectFlags);

	VkDescriptorSetLayoutBinding descriptorset_layout_binding(
		VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t binding);

	VkWriteDescriptorSet
	write_descriptor_buffer(VkDescriptorType type, VkDescriptorSet dstSet,
							VkDescriptorBufferInfo* bufferInfo,
							uint32_t binding);

	VkSamplerCreateInfo sampler_create_info(VkFilter filters, VkSamplerAddressMode samplerAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
	
	VkWriteDescriptorSet write_descriptor_image(VkDescriptorType type, VkDescriptorSet dstSet, VkDescriptorImageInfo* imageInfo, uint32_t binding);

	enum class ShaderType : u64 { VERTEX = 0x00000001, FRAGMENT = 0x00000010 };

	struct Shader {
		VkShaderModule module;
		ShaderType type;
	};

	struct VertexInputDescription {
		ArrayList<VkVertexInputBindingDescription> bindings;
		ArrayList<VkVertexInputAttributeDescription> attributes;
		VkPipelineVertexInputStateCreateFlags flags{0};
	};

	class PipelineBuilder {
	private:
		// @TODO: perhaps give them all reasonable reserve
		ArrayList<Shader> mShaders{};

		ArrayList<VkPipelineShaderStageCreateInfo> mShaderStages{};

		ArrayList<VkVertexInputBindingDescription> mVertexBindings{};

		ArrayList<VkVertexInputAttributeDescription> mVertexAttributes{};

		ArrayList<VkDynamicState> mDynamicStates{};

		ArrayList<VkViewport> mViewports{};

		ArrayList<VkRect2D> mScissors{};

		ArrayList<VkPipelineColorBlendAttachmentState> mColorBlendAttachments{};

		ArrayList<VkPushConstantRange> mPushConstants{};

		ArrayList<VkDescriptorSetLayout> mDescriptorSetLayouts;

		VkPipelineInputAssemblyStateCreateInfo mInputAssembly{
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false};

		VkPipelineRasterizationStateCreateInfo mRasterizer{
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
			1.f};

		VkPipelineMultisampleStateCreateInfo mMultisampling{
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			nullptr,
			0,
			VK_SAMPLE_COUNT_1_BIT,
			false,
			1.f,
			nullptr,
			0,
			0};

		VkPipelineDepthStencilStateCreateInfo mDepthStencil{
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

		VkPipelineColorBlendStateCreateInfo mColorBlendState{
			// @TODO: add color attachments array_list
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			nullptr,
			0,
			false,
			VK_LOGIC_OP_COPY,
			0,
			nullptr,
			{0.f, 0.f, 0.f, 0.f}};

		VkPipelineLayoutCreateInfo mLayoutCi{
			// @TODO: add methods to add more descriptor sets and push
			// constants
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			nullptr,
			0,
			0,
			nullptr,
			0,
			nullptr};

		VkPipelineDynamicStateCreateInfo mDynamicStateCis{
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 0,
			nullptr};

		VkPipelineLayout mPipelineLayout{};

	public:
		PipelineBuilder& add_shader(VkDevice device, const char* path,
									ShaderType type);

		PipelineBuilder& add_vertex_binding(u32 stride,
											VkVertexInputRate input_rate);

		PipelineBuilder& add_vertex_attribute(u32 binding, u32 location,
											  VkFormat format, u32 offset);

		PipelineBuilder&
		set_vertex_input_description(VertexInputDescription&& desc);

		PipelineBuilder& set_input_assembly(VkPrimitiveTopology topology,
											bool primitive_restart_enable);

		PipelineBuilder& set_cull_mode(VkCullModeFlags cull_mode,
									   VkFrontFace front_face);

		PipelineBuilder& set_polygon_mode(VkPolygonMode mode);

		PipelineBuilder& add_default_color_blend_attachment();

		PipelineBuilder& set_multisampling_enabled(
			bool val, VkSampleCountFlagBits count = VK_SAMPLE_COUNT_1_BIT);

		PipelineBuilder&
		set_depth_testing(bool testEnabled, bool depthWrite,
						  VkCompareOp compare_op = VK_COMPARE_OP_LESS_OR_EQUAL);

		PipelineBuilder&
		set_color_blending_enabled(bool enabled,
								   VkLogicOp op = VK_LOGIC_OP_COPY);

		PipelineBuilder& add_dynamic_state(VkDynamicState dynamic_state);

		PipelineBuilder& add_viewport(VkViewport viewport);

		PipelineBuilder& add_scissor(VkRect2D scissor);

		PipelineBuilder& add_push_constant(u32 size,
										   VkPipelineStageFlags stages);

		PipelineBuilder&
		add_descriptor_set_layout(VkDescriptorSetLayout layout);

		VkPipelineLayout build_layout(VkDevice device);

		VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
	};
} // namespace vkbuild
