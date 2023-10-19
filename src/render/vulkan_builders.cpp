#include "vulkan_builders.h"
#include "vulkan/vulkan_core.h"

namespace vkbuild {

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

	VkSubmitInfo submit_info(VkCommandBuffer *buf) {
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

	PipelineBuilder &
	PipelineBuilder::add_shader_stage(VkPipelineShaderStageCreateInfo ci) {
		_shader_stages.push_back(ci);
		return *this;
	}

	PipelineBuilder &
	PipelineBuilder::add_vertex_binding(u32 stride,
										VkVertexInputRate input_rate) {
		VkVertexInputBindingDescription desc{
			static_cast<u32>(_vertex_bindings.size()), stride, input_rate};
		_vertex_bindings.push_back(desc);
		return *this;
	}

	PipelineBuilder &PipelineBuilder::add_vertex_attribute(u32 binding,
														   u32 location,
														   VkFormat format,
														   u32 offset) {
		VkVertexInputAttributeDescription desc{location, binding, format,
											   offset};
		_vertex_attributes.push_back(desc);
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_input_assembly(
		VkPrimitiveTopology topology,
		VkPipelineInputAssemblyStateCreateFlags create_flags,
		bool primitive_restart_enable) {
		_input_assembly.topology = topology;
		_input_assembly.flags = create_flags;
		_input_assembly.primitiveRestartEnable = primitive_restart_enable;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_viewport(VkViewport viewport) {
		_viewport = viewport;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_scissor(VkRect2D scissor) {
		_scissor = scissor;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode,
													VkFrontFace front_face) {
		_rasterizer.cullMode = cull_mode;
		_rasterizer.frontFace = front_face;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
		_rasterizer.polygonMode = mode;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_color_blend_enabled(bool val) {
		_color_blend_attachment.blendEnable = val;
		return *this;
	}

	PipelineBuilder &
	PipelineBuilder::set_multisampling_enabled(bool val,
											   VkSampleCountFlagBits count) {
		_multisampling.sampleShadingEnable = val;
		_multisampling.rasterizationSamples = count;
		return *this;
	}

	PipelineBuilder &
	PipelineBuilder::set_depth_testing(bool testEnabled, bool depthWrite,
									   VkCompareOp compare_op) {
		_depth_stencil.depthTestEnable = testEnabled;
		_depth_stencil.depthWriteEnable = depthWrite;
		_depth_stencil.depthCompareOp = compare_op;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_color_blending_enabled(bool enabled,
																 VkLogicOp op) {
		_color_blend_state.logicOpEnable = enabled;
		_color_blend_state.logicOp = op;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::add_dynamic_state(VkDynamicState dynamic_state){
		_dynamic_states.push_back(dynamic_state);
		_dynamic_state_cis.pDynamicStates = _dynamic_states.data();
		_dynamic_state_cis.dynamicStateCount = static_cast<u32>(_dynamic_states.size());
		return *this;
	}

	VkPipelineLayout PipelineBuilder::build_layout() {}

	VkPipeline PipelineBuilder::build_pipeline(VkDevice device,
											   VkRenderPass pass) {
		VkPipelineVertexInputStateCreateInfo vertex_input = {
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<u32>(_vertex_bindings.size()),
			_vertex_bindings.data(),
			static_cast<u32>(_vertex_attributes.size()),
			_vertex_attributes.data()};
	}
} // namespace vkbuild
