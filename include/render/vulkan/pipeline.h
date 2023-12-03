#pragma once
#include "render/vulkan/shader.h"
#include "render/vulkan/types.h"

namespace render::vulkan::builder {

	class PipelineBuilder {
	public:
		PipelineBuilder& add_shader_module(ShaderModule*, ShaderType type);

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

		PipelineBuilder& set_shaders(render::vulkan::ShaderSet* set);

		inline VkPipelineLayout layout() const { return mPipelineLayout; }

	private:
		// @TODO: perhaps give them all reasonable reserve
		ArrayList<Shader> mShaders{};

		ArrayList<render::vulkan::ShaderModule*> mShaderModules{};

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
	};
} // namespace render::vulkan::builder
