#include "render/vulkan/pipeline.h"
#include <algorithm>
#include <fstream>

namespace render::vulkan::builder{

	PipelineBuilder&
	PipelineBuilder::add_shader_module(ShaderModule* module, ShaderType type) {
		VkPipelineShaderStageCreateInfo stage_ci{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			static_cast<VkShaderStageFlagBits>(type),
			module->handle(),
			"main"};
		mShaderStages.push_back(stage_ci);
        return *this;
    }

	PipelineBuilder& PipelineBuilder::add_shader(VkDevice device,
												 const char* path,
												 ShaderType type) {
		for (auto& shader : mShaders) {
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
			core::Logger::Trace("Failed to create shader module. {}", (int)res);
			return *this;
		}
		mShaders.push_back(shader);
		VkPipelineShaderStageCreateInfo stage_ci{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			static_cast<VkShaderStageFlagBits>(type),
			shader.module,
			"main"};
		mShaderStages.push_back(stage_ci);
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::set_shaders(render::vulkan::ShaderSet* set) {
		mShaderStages.clear();
		mShaderStages = set->pipeline_stages();
		mPipelineLayout = set->pipeline_layout();
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::add_vertex_binding(u32 stride,
										VkVertexInputRate input_rate) {
		VkVertexInputBindingDescription desc{
			static_cast<u32>(mVertexBindings.size()), stride, input_rate};
		mVertexBindings.push_back(desc);
		return *this;
	}

	PipelineBuilder& PipelineBuilder::add_vertex_attribute(u32 binding,
														   u32 location,
														   VkFormat format,
														   u32 offset) {
		VkVertexInputAttributeDescription desc{location, binding, format,
											   offset};
		mVertexAttributes.push_back(desc);
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_vertex_input_description(
		VertexInputDescription&& desc) {
		mVertexBindings = std::move(desc.bindings);
		mVertexAttributes = std::move(desc.attributes);
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::set_input_assembly(VkPrimitiveTopology topology,
										bool primitive_restart_enable) {
		mInputAssembly.topology = topology;
		mInputAssembly.primitiveRestartEnable = primitive_restart_enable;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode,
													VkFrontFace front_face) {
		mRasterizer.cullMode = cull_mode;
		mRasterizer.frontFace = front_face;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_polygon_mode(VkPolygonMode mode) {
		mRasterizer.polygonMode = mode;
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::set_multisampling_enabled(bool val,
											   VkSampleCountFlagBits count) {
		mMultisampling.sampleShadingEnable = val;
		mMultisampling.rasterizationSamples = count;
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::set_depth_testing(bool testEnabled, bool depthWrite,
									   VkCompareOp compare_op) {
		mDepthStencil.depthTestEnable = testEnabled;
		mDepthStencil.depthWriteEnable = depthWrite;
		mDepthStencil.depthCompareOp = compare_op;
		return *this;
	}

	PipelineBuilder& PipelineBuilder::set_color_blending_enabled(bool enabled,
																 VkLogicOp op) {
		mColorBlendState.logicOpEnable = enabled;
		mColorBlendState.logicOp = op;
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
		mColorBlendAttachments.push_back(color_blend_attachment);
		mColorBlendState.pAttachments = mColorBlendAttachments.data();
		mColorBlendState.attachmentCount =
			static_cast<int>(mColorBlendAttachments.size());
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::add_dynamic_state(VkDynamicState dynamic_state) {
		mDynamicStates.push_back(dynamic_state);
		mDynamicStateCis.pDynamicStates = mDynamicStates.data();
		mDynamicStateCis.dynamicStateCount =
			static_cast<u32>(mDynamicStates.size());
		return *this;
	}

	PipelineBuilder& PipelineBuilder::add_viewport(VkViewport viewport) {
		mViewports.push_back(viewport);
		return *this;
	}

	PipelineBuilder& PipelineBuilder::add_scissor(VkRect2D scissor) {
		mScissors.push_back(scissor);
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::add_push_constant(u32 size, VkPipelineStageFlags stages) {
		VkPushConstantRange pc{stages};
		for (auto& c : mPushConstants) {
			pc.offset += c.size;
		}
		pc.size = size;
		mPushConstants.push_back(pc);
		return *this;
	}

	PipelineBuilder&
	PipelineBuilder::add_descriptor_set_layout(VkDescriptorSetLayout layout) {
		mDescriptorSetLayouts.push_back(layout);
		return *this;
	}

	VkPipelineLayout PipelineBuilder::build_layout(VkDevice device) {
		mLayoutCi.pushConstantRangeCount =
			static_cast<u32>(mPushConstants.size());
		mLayoutCi.pPushConstantRanges = mPushConstants.data();
		mLayoutCi.setLayoutCount =
			static_cast<u32>(mDescriptorSetLayouts.size());
		mLayoutCi.pSetLayouts = mDescriptorSetLayouts.data();
		VK_CHECK(vkCreatePipelineLayout(device, &mLayoutCi, nullptr,
										&mPipelineLayout));
		core::Logger::Trace("Pipeline LAYOUT successfully created.");
		return mPipelineLayout;
	}

	VkPipeline PipelineBuilder::build_pipeline(VkDevice device,
											   VkRenderPass pass) {

		VkPipelineVertexInputStateCreateInfo vertex_input = {
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<u32>(mVertexBindings.size()),
			mVertexBindings.data(),
			static_cast<u32>(mVertexAttributes.size()),
			mVertexAttributes.data()};

		VkPipelineViewportStateCreateInfo viewport_ci{
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			nullptr,
			0,
			static_cast<u32>(mViewports.size()),
			mViewports.data(),
			static_cast<u32>(mScissors.size()),
			mScissors.data()};

		VkGraphicsPipelineCreateInfo pipeline_ci{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			nullptr,
			0,
			static_cast<u32>(mShaderStages.size()),
			mShaderStages.data(),
			&vertex_input,
			&mInputAssembly,
			nullptr, // @TODO handle tesselation stage
			&viewport_ci,
			&mRasterizer,
			&mMultisampling,
			&mDepthStencil,
			&mColorBlendState,
			&mDynamicStateCis,
			mPipelineLayout,
			pass,
			0,
			nullptr,
			-1,
		};

		VkPipeline pipeline;
		VK_CHECK(vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_ci,
										   nullptr, &pipeline));
		core::Logger::Trace("Pipeline successfully created.");
		for (auto& shader : mShaders) {
			vkDestroyShaderModule(device, shader.module, nullptr);
		}
		return pipeline;
	}

}
