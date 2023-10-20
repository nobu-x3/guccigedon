#include "vulkan_builders.h"
#include <algorithm>
#include <fstream>
#include "vulkan/vulkan_core.h"
#include "vulkan_types.h"

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

	PipelineBuilder& PipelineBuilder::add_shader(VkDevice device,
												 const char* path,
												 ShaderType type) {
		for (auto& shader : _shaders) {
			if (shader.type == type) {
#define PROCESS_VAL(p)                                                         \
	case (p):                                                                  \
		core::Logger::Error(                                                   \
			"Attempting to add a second shader of the type %s", #p);           \
		return *this;
				switch (type) {
					PROCESS_VAL(ShaderType::VERTEX)
					PROCESS_VAL(ShaderType::FRAGMENT)
				}
#undef PROCESS_VAL
			}
		}
		// open the file. With cursor at the end
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			core::Logger::Error("Failed to open shader code at path %s", path);
			return *this;
		}
		// find what the size of the file is by looking up the location of
		// the cursor because the cursor is at the end, it gives the size
		// directly in bytes
		size_t file_size = (size_t)file.tellg();
		// spirv expects the buffer to be on uint32, so make sure to reserve
		// a int vector big enough for the entire file
		ArrayList<u32> buffer(file_size / sizeof(u32));
		// put file cursor at beggining
		file.seekg(0);
		// load the entire file into the buffer
		file.read((char*)buffer.data(), file_size);
		// now that the file is loaded into the buffer, we can close it
		file.close();
		// create a new shader module, using the buffer we loaded
		// codeSize has to be in bytes, so multply the ints in the buffer by
		// size of int to know the real size of the buffer
		VkShaderModuleCreateInfo createInfo = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0,
			buffer.size() * sizeof(u32), buffer.data()};
		Shader shader{0, type};
		// check that the creation goes well.
		VkResult res =
			vkCreateShaderModule(device, &createInfo, nullptr, &shader.module);
		if (res != VK_SUCCESS) {
			core::Logger::Trace("Failed to create shader module. %d", res);
			return *this;
		}
		_shaders.push_back(shader);
		VkPipelineShaderStageCreateInfo stage_ci{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			static_cast<VkShaderStageFlagBits>(type),
			shader.module,
			"main"};
		_shader_stages.push_back(stage_ci);
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::add_vertex_binding(u32 stride,
										VkVertexInputRate input_rate) {
		VkVertexInputBindingDescription desc{
			static_cast<u32>(_vertex_bindings.size()), stride, input_rate};
		_vertex_bindings.push_back(desc);
		return *this;
	}

	PipelineBuilder& PipelineBuilder::add_vertex_attribute(u32 binding,
														   u32 location,
														   VkFormat format,
														   u32 offset) {
		VkVertexInputAttributeDescription desc{location, binding, format,
											   offset};
		_vertex_attributes.push_back(desc);
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_vertex_input_description(
		VertexInputDescription&& desc) {
		_vertex_bindings = std::move(desc.bindings);
		_vertex_attributes = std::move(desc.attributes);
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::set_input_assembly(VkPrimitiveTopology topology,
										bool primitive_restart_enable) {
		_input_assembly.topology = topology;
		_input_assembly.primitiveRestartEnable = primitive_restart_enable;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode,
													VkFrontFace front_face) {
		_rasterizer.cullMode = cull_mode;
		_rasterizer.frontFace = front_face;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
		_rasterizer.polygonMode = mode;
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::set_multisampling_enabled(bool val,
											   VkSampleCountFlagBits count) {
		_multisampling.sampleShadingEnable = val;
		_multisampling.rasterizationSamples = count;
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::set_depth_testing(bool testEnabled, bool depthWrite,
									   VkCompareOp compare_op) {
		_depth_stencil.depthTestEnable = testEnabled;
		_depth_stencil.depthWriteEnable = depthWrite;
		_depth_stencil.depthCompareOp = compare_op;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_color_blending_enabled(bool enabled,
																 VkLogicOp op) {
		_color_blend_state.logicOpEnable = enabled;
		_color_blend_state.logicOp = op;
		return *this;
	}
	PipelineBuilder& PipelineBuilder::add_default_color_blend_attachment() {
		VkPipelineColorBlendAttachmentState color_blend_attachment{
			false,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_SRC_ALPHA,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
				VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
		_color_blend_attachments.push_back(color_blend_attachment);
		_color_blend_state.pAttachments = _color_blend_attachments.data();
		_color_blend_state.attachmentCount =
			static_cast<int>(_color_blend_attachments.size());
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::add_dynamic_state(VkDynamicState dynamic_state) {
		_dynamic_states.push_back(dynamic_state);
		_dynamic_state_cis.pDynamicStates = _dynamic_states.data();
		_dynamic_state_cis.dynamicStateCount =
			static_cast<u32>(_dynamic_states.size());
		return *this;
	}

	PipelineBuilder& PipelineBuilder::add_viewport(VkViewport viewport) {
		_viewports.push_back(viewport);
		return *this;
	}

	PipelineBuilder& PipelineBuilder::add_scissor(VkRect2D scissor) {
		_scissors.push_back(scissor);
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::add_push_constant(u32 size, VkPipelineStageFlags stages) {
        VkPushConstantRange pc {stages};
        for(auto& c : _push_constants){
            pc.offset += c.size;
        }
        pc.size = size;
        _push_constants.push_back(pc);
        return *this;
    }

	VkPipelineLayout PipelineBuilder::build_layout(VkDevice device) {
        _layout_ci.pushConstantRangeCount = static_cast<u32>(_push_constants.size());
        _layout_ci.pPushConstantRanges = _push_constants.data();
		VK_CHECK(vkCreatePipelineLayout(device, &_layout_ci, nullptr,
										&_pipeline_layout));
		core::Logger::Trace("Pipeline LAYOUT successfully created.");
		return _pipeline_layout;
	}

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

		VkPipelineViewportStateCreateInfo viewport_ci{
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<u32>(_viewports.size()),
			_viewports.data(),
			static_cast<u32>(_scissors.size()),
			_scissors.data()};

		VkGraphicsPipelineCreateInfo pipeline_ci{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
			0,
			static_cast<u32>(_shader_stages.size()),
			_shader_stages.data(),
			&vertex_input,
			&_input_assembly,
			nullptr, // @TODO handle tesselation stage
			&viewport_ci,
			&_rasterizer,
			&_multisampling,
			&_depth_stencil,
			&_color_blend_state,
			&_dynamic_state_cis,
			_pipeline_layout,
			pass,
			0,
			nullptr,
			-1,
		};

		VkPipeline pipeline;
		VK_CHECK(vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_ci,
										   nullptr, &pipeline));
		core::Logger::Trace("Pipeline successfully created.");
		for (auto& shader : _shaders) {
			vkDestroyShaderModule(device, shader.module, nullptr);
		}
		return pipeline;
	}
} // namespace vkbuild
