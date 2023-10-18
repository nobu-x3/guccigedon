#include "renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../../vendor/vk-bootstrap/src/VkBootstrap.h"
#include "../core/logger.h"
#include "vulkan_builders.h"
#include "vulkan_types.h"

namespace render {

	VulkanRenderer::VulkanRenderer() {
		SDL_Init(SDL_INIT_VIDEO);
		_p_window = SDL_CreateWindow("Guccigedon", SDL_WINDOWPOS_CENTERED,
									SDL_WINDOWPOS_CENTERED,
									_window_extent.width, _window_extent.height,
									(SDL_WindowFlags)(SDL_WINDOW_VULKAN));
		if (!_p_window) {
			core::Logger::Fatal("Failed to create a window. %s",
								SDL_GetError());
			exit(-1);
		}
		init_instance();
		init_swapchain();
		init_default_renderpass();
		init_framebuffers();
		init_commands();
		init_sync_objects();
	}

	VulkanRenderer::~VulkanRenderer() {
		vkDeviceWaitIdle(_device);
		vkDestroyCommandPool(_device, _command_pool, nullptr);
		vkDestroyFence(_device, _render_fence, nullptr);
		vkDestroySemaphore(_device, _present_semaphore, nullptr);
		vkDestroySemaphore(_device, _render_semaphore, nullptr);
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		vkDestroyRenderPass(_device, _render_pass, nullptr);
		for (int i = 0; i < _framebuffers.size(); ++i) {
			vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			vkDestroyImageView(_device, _swapchain_image_views[i], nullptr);
		}
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkDestroyDevice(_device, nullptr);
		vkb::destroy_debug_utils_messenger(_instance, _debug_msger);
		vkDestroyInstance(_instance, nullptr);
		SDL_DestroyWindow(_p_window);
	}

	void VulkanRenderer::draw() {
        // Not rendering when minimized
        if(SDL_GetWindowFlags(_p_window) & SDL_WINDOW_MINIMIZED){
            return;
        }
        VK_CHECK(vkWaitForFences(_device, 1, &_render_fence, true, 1000000000));
        VK_CHECK(vkResetFences(_device, 1, &_render_fence));
        VK_CHECK(vkResetCommandBuffer(_main_command_buffer, 0));
        u32 image_index{};
        VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _present_semaphore, nullptr, &image_index));
        VkCommandBuffer buf = _main_command_buffer;
        VkCommandBufferBeginInfo buf_begin_info = vkbuild::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VK_CHECK(vkBeginCommandBuffer(buf, &buf_begin_info));
        VkClearValue clear;
        static u64 frame_nr{};
        VkRenderPassBeginInfo renderpass_info = vkbuild::renderpass_begin_info(_render_pass, _window_extent, _framebuffers[image_index]);
        float flash = abs(sin(frame_nr/120.f));
        clear.color = {{0.f, 0.f, flash, 1.f}};
        renderpass_info.clearValueCount = 1;
        renderpass_info.pClearValues = &clear;
        vkCmdBeginRenderPass(buf, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
        // insert actual commands
        //
        vkCmdEndRenderPass(buf);
        VK_CHECK(vkEndCommandBuffer(buf));
        VkSubmitInfo submit = vkbuild::submit_info(&buf);
        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit.pWaitDstStageMask=&wait_stage;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &_present_semaphore;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &_render_semaphore;
        VK_CHECK(vkQueueSubmit(_graphics_queue, 1, &submit, _render_fence));
        VkPresentInfoKHR present_info = vkbuild::present_info();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &_swapchain;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &_render_semaphore;
        present_info.pImageIndices = &image_index;
        VK_CHECK(vkQueuePresentKHR(_graphics_queue, &present_info));
        ++frame_nr;
    }

	void VulkanRenderer::run() {
		SDL_Event e;
		bool quit = false;
		while (!quit) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT)
					quit = true;
			}
			draw();
		}
	}

	void VulkanRenderer::init_instance() {
		// all this vkb stuff is nice and all but eventually we will make
		// something more robust manually. There's a ton of unsafe stuff here.
		vkb::InstanceBuilder builder;
		vkb::Instance vkb_inst = builder.set_app_name("Guccigedon")
									 .request_validation_layers(true)
									 .use_default_debug_messenger()
									 .require_api_version(1, 1, 0)
									 .build()
									 .value();
		_instance = vkb_inst.instance;
		_debug_msger = vkb_inst.debug_messenger;
		SDL_Vulkan_CreateSurface(_p_window, _instance, &_surface);
		vkb::PhysicalDeviceSelector selector{vkb_inst};
		vkb::PhysicalDevice vkb_phys_dev = selector.set_minimum_version(1, 1)
											   .set_surface(_surface)
											   .select()
											   .value();
		vkb::DeviceBuilder dev_builder{vkb_phys_dev};
		vkb::Device vkb_dev = dev_builder.build().value();
		_device = vkb_dev.device;
		_physical_device = vkb_phys_dev.physical_device;
		_graphics_queue = vkb_dev.get_queue(vkb::QueueType::graphics).value();
		_graphics_queue_family =
			vkb_dev.get_queue_index(vkb::QueueType::graphics).value();
	}

	void VulkanRenderer::init_swapchain() {
		vkb::SwapchainBuilder swap_buider{_physical_device, _device, _surface};
		vkb::Swapchain vkb_swap =
			swap_buider.use_default_format_selection()
				.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
				.set_desired_extent(_window_extent.width, _window_extent.height)
				.build()
				.value();
		_swapchain = vkb_swap.swapchain;
		_swapchain_images = vkb_swap.get_images().value();
		_swapchain_image_views = vkb_swap.get_image_views().value();
		_swapchain_image_format = vkb_swap.image_format;
	}

	void VulkanRenderer::init_commands() {
		VkCommandPoolCreateInfo command_pool_ci = vkbuild::command_pool_ci(
			_graphics_queue_family,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VK_CHECK(vkCreateCommandPool(_device, &command_pool_ci, nullptr,
									 &_command_pool));
		VkCommandBufferAllocateInfo alloc_info =
			vkbuild::command_buffer_ai(_command_pool, 1);
		VK_CHECK(vkAllocateCommandBuffers(_device, &alloc_info,
										  &_main_command_buffer));
	}

	void VulkanRenderer::init_framebuffers() {
		VkFramebufferCreateInfo fb_ci =
			vkbuild::framebuffer_ci(_render_pass, _window_extent);
		const u32 image_count = _swapchain_images.size();
		_framebuffers = std::vector<VkFramebuffer>(image_count);
		for (int i = 0; i < image_count; ++i) {
			fb_ci.pAttachments = &_swapchain_image_views[i];
			VK_CHECK(vkCreateFramebuffer(_device, &fb_ci, nullptr,
										 &_framebuffers[i]));
		}
	}

	void VulkanRenderer::init_default_renderpass() {
		VkAttachmentDescription color_attachment = {};
		color_attachment.format = _swapchain_image_format;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// we are going to create 1 subpass, which is the minimum you can do
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		// 1 dependency, which is from "outside" into the subpass. And we can
		// read or write color

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment;
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 1;
		render_pass_info.pDependencies = &dependency;
		VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr,
									&_render_pass));
	}

	void VulkanRenderer::init_sync_objects() {
		// create syncronization structures
		// one fence to control when the gpu has finished rendering the frame,
		// and 2 semaphores to syncronize rendering with swapchain
		// we want the fence to start signalled so we can wait on it on the
		// first frame
		VkFenceCreateInfo fenceCreateInfo =
			vkbuild::fence_ci(VK_FENCE_CREATE_SIGNALED_BIT);

		VK_CHECK(
			vkCreateFence(_device, &fenceCreateInfo, nullptr, &_render_fence));

		VkSemaphoreCreateInfo semaphoreCreateInfo = vkbuild::semaphore_ci();

		VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
								   &_present_semaphore));
		VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
								   &_render_semaphore));
	}
} // namespace render
