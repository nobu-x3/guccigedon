#include "vulkan_builders.h"

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
	PipelineBuilder::set_vertex_input(VkPipelineVertexInputStateCreateInfo ci) {
		_vertex_input = ci;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_input_assembly(
		VkPipelineInputAssemblyStateCreateInfo ci){
			_input_assembly = ci;
			return *this;
		}

	PipelineBuilder &PipelineBuilder::set_viewport(VkViewport viewport){
		_viewport = viewport;
		return *this;
	}

	PipelineBuilder &PipelineBuilder::set_scissor(VkRect2D scissor);

	PipelineBuilder &
	PipelineBuilder::set_rasterizer(VkPipelineRasterizationStateCreateInfo ci);

	PipelineBuilder &
	PipelineBuilder::set_color_blend(VkPipelineColorBlendAttachmentState state);

	PipelineBuilder &
	PipelineBuilder::set_multisampling(VkPipelineMultisampleStateCreateInfo ci);

	VkPipelineLayout PipelineBuilder::build_layout();

	VkPipeline PipelineBuilder::build_pipeline(VkDevice device,
											   VkRenderPass pass);
} // namespace vkbuild
