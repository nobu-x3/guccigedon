#include "renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <vulkan/vulkan_core.h>
#include "../../vendor/vk-bootstrap/src/VkBootstrap.h"
#include "../core/logger.h"
#include "vulkan_builders.h"
#include "vulkan_types.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace render {
	VulkanRenderer::VulkanRenderer() {
		SDL_Init(SDL_INIT_VIDEO);
		mpWindow = SDL_CreateWindow("Guccigedon", SDL_WINDOWPOS_CENTERED,
									SDL_WINDOWPOS_CENTERED, mWindowExtent.width,
									mWindowExtent.height,
									(SDL_WindowFlags)(SDL_WINDOW_VULKAN));
		if (!mpWindow) {
			core::Logger::Fatal("Failed to create a window. %s",
								SDL_GetError());
			exit(-1);
		}
		init_instance();
		// initialize the memory allocator
		VmaAllocatorCreateInfo allocator_ci = {};
		allocator_ci.physicalDevice = mPhysicalDevice;
		allocator_ci.device = mDevice;
		allocator_ci.instance = mInstance;
		vmaCreateAllocator(&allocator_ci, &mAllocator);
		init_swapchain();
		init_default_renderpass();
		init_framebuffers();
		init_commands();
		init_sync_objects();
		init_pipeline();
		load_mesh();
	}

	VulkanRenderer::~VulkanRenderer() {
		// this stuff is sort of unsafe, so need to check for handles > 0 before
		// destroying, otherwise will segfault
		if (mDevice) {
			vkDeviceWaitIdle(mDevice);
			if (mCommandPool) {
				vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
			}
			if (mRenderFence) {
				vkDestroyFence(mDevice, mRenderFence, nullptr);
			}
			if (mPresentSemaphore) {
				vkDestroySemaphore(mDevice, mPresentSemaphore, nullptr);
			}
			if (mRenderSemaphore) {
				vkDestroySemaphore(mDevice, mRenderSemaphore, nullptr);
			}
			if (mSwapchain) {
				vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
			}
			if (mRenderPass) {
				vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
			}
			if (mGraphicsPipelineLayout) {
				vkDestroyPipelineLayout(mDevice, mGraphicsPipelineLayout,
										nullptr);
			}
			if (mGraphicsPipeline) {
				vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
			}
			for (int i = 0; i < mFramebuffers.size(); ++i) {
				if (mFramebuffers[i])
					vkDestroyFramebuffer(mDevice, mFramebuffers[i], nullptr);
				if (mSwapchainImageViews[i])
					vkDestroyImageView(mDevice, mSwapchainImageViews[i],
									   nullptr);
			}
			mMesh.deinit(mAllocator);
			mMonkeyMesh.deinit(mAllocator);
            mDepthAttachment.destroy();
			vmaDestroyAllocator(mAllocator);
			vkDestroyDevice(mDevice, nullptr);
		}
		if (mInstance) {
			if (mSurface) {
				vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
			}
			if (fpDebugMsger) {
				vkb::destroy_debug_utils_messenger(mInstance, fpDebugMsger);
			}
			vkDestroyInstance(mInstance, nullptr);
		}
		if (mpWindow) {
			SDL_DestroyWindow(mpWindow);
		}
	}

	void VulkanRenderer::draw() {
		// Not rendering when minimized
		if (SDL_GetWindowFlags(mpWindow) & SDL_WINDOW_MINIMIZED) {
			return;
		}
		// wait until last frame is rendered, timeout 1s
		VK_CHECK(vkWaitForFences(mDevice, 1, &mRenderFence, true, 1000000000));
		VK_CHECK(vkResetFences(mDevice, 1, &mRenderFence));
		// now can reset command buffer safely
		VK_CHECK(vkResetCommandBuffer(mMainCommandBuffer, 0));
		u32 image_index{};
		VK_CHECK(vkAcquireNextImageKHR(mDevice, mSwapchain, 1000000000,
									   mPresentSemaphore, nullptr,
									   &image_index));
		VkCommandBuffer buf = mMainCommandBuffer;
		VkCommandBufferBeginInfo buf_begin_info =
			vkbuild::command_buffer_begin_info(
				VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(buf, &buf_begin_info));
		VkClearValue color_clear{};
        VkClearValue depth_clear{};
        depth_clear.depthStencil.depth = 1.f;
		static u64 frame_nr{};

		VkRenderPassBeginInfo renderpass_info = vkbuild::renderpass_begin_info(
			mRenderPass, mWindowExtent, mFramebuffers[image_index]);
		float flash = abs(sin(frame_nr / 120.f));
		color_clear.color = {{0.f, 0.f, flash, 1.f}};
        VkClearValue clear_values[] {color_clear, depth_clear};
		renderpass_info.clearValueCount = 2;
		renderpass_info.pClearValues = &clear_values[0];
		vkCmdBeginRenderPass(buf, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
		// insert actual commands
		vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
						  mGraphicsPipeline);
		VkDeviceSize offset{0};
		vkCmdBindVertexBuffers(buf, 0, 1, &mMonkeyMesh.buffer.handle, &offset);
		// make a model view matrix for rendering the object
		// camera position
		glm::vec3 camPos = {0.f, 0.f, -2.f};

		glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
		// camera projection
		glm::mat4 projection = glm::perspective(
			glm::radians(70.f),
			static_cast<f32>(mWindowExtent.width) / mWindowExtent.height, 0.1f,
			200.0f);
		projection[1][1] *= -1;
		// model rotation
		glm::mat4 model = glm::rotate(
			glm::mat4{1.0f}, glm::radians(frame_nr * 0.4f), glm::vec3(0, 1, 0));

		// calculate final mesh matrix
		glm::mat4 mesh_matrix = projection * view * model;

		MeshPushConstant constants;
		constants.render_matrix = mesh_matrix;

		// upload the matrix to the GPU via push constants
		vkCmdPushConstants(buf, mGraphicsPipelineLayout,
						   VK_SHADER_STAGE_VERTEX_BIT, 0,
						   sizeof(MeshPushConstant), &constants);
		vkCmdDraw(buf, mMonkeyMesh.vertices.size(), 1, 0, 0);
		vkCmdEndRenderPass(buf);
		VK_CHECK(vkEndCommandBuffer(buf));
		// waiting on _present_semaphore which is signaled when swapchain is
		// ready. signal _render_semaphore when finished rendering.
		VkSubmitInfo submit = vkbuild::submit_info(&buf);
		VkPipelineStageFlags wait_stage =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit.pWaitDstStageMask = &wait_stage;
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &mPresentSemaphore;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &mRenderSemaphore;
		// render fence blocks until commands finish executing
		VK_CHECK(vkQueueSubmit(mGraphicsQueue, 1, &submit, mRenderFence));

		// waiting on rendering to finish, then presenting
		VkPresentInfoKHR present_info = vkbuild::present_info();
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &mSwapchain;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &mRenderSemaphore;
		present_info.pImageIndices = &image_index;
		VK_CHECK(vkQueuePresentKHR(mGraphicsQueue, &present_info));
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
		mInstance = vkb_inst.instance;
		fpDebugMsger = vkb_inst.debug_messenger;
		SDL_Vulkan_CreateSurface(mpWindow, mInstance, &mSurface);
		vkb::PhysicalDeviceSelector selector{vkb_inst};
		vkb::PhysicalDevice vkb_phys_dev = selector.set_minimum_version(1, 1)
											   .set_surface(mSurface)
											   .select()
											   .value();
		vkb::DeviceBuilder dev_builder{vkb_phys_dev};
		vkb::Device vkb_dev = dev_builder.build().value();
		mDevice = vkb_dev.device;
		mPhysicalDevice = vkb_phys_dev.physical_device;
		mGraphicsQueue = vkb_dev.get_queue(vkb::QueueType::graphics).value();
		mGraphicsQueueFamily =
			vkb_dev.get_queue_index(vkb::QueueType::graphics).value();
	}

	void VulkanRenderer::init_swapchain() {
		vkb::SwapchainBuilder swap_buider{mPhysicalDevice, mDevice, mSurface};
		vkb::Swapchain vkb_swap =
			swap_buider.use_default_format_selection()
				.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
				.set_desired_extent(mWindowExtent.width, mWindowExtent.height)
				.build()
				.value();
		mSwapchain = vkb_swap.swapchain;
		mSwapchainImages = vkb_swap.get_images().value();
		mSwapchainImageViews = vkb_swap.get_image_views().value();
		mSwapchainImageFormat = vkb_swap.image_format;
		VkExtent3D depthImageExtent = {mWindowExtent.width,
									   mWindowExtent.height, 1};
        VmaAllocationCreateInfo imgAllocCi {};
        imgAllocCi.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        imgAllocCi.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		auto res = Image::create_image(
			mAllocator, mDevice,
			vkbuild::image_ci(mDepthFormat,
							  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
							  depthImageExtent),
            imgAllocCi
			);
        if(res.has_value()){
            bool result = res->create_view(vkbuild::imageview_ci(mDepthFormat, res->handle, VK_IMAGE_ASPECT_DEPTH_BIT));
            if (!result){
                return;
            }
            mDepthAttachment = std::move(res.value());
        }
	}

	void VulkanRenderer::init_commands() {
		VkCommandPoolCreateInfo command_pool_ci = vkbuild::command_pool_ci(
			mGraphicsQueueFamily,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		VK_CHECK(vkCreateCommandPool(mDevice, &command_pool_ci, nullptr,
									 &mCommandPool));
		VkCommandBufferAllocateInfo alloc_info =
			vkbuild::command_buffer_ai(mCommandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(mDevice, &alloc_info,
										  &mMainCommandBuffer));
	}

	void VulkanRenderer::init_framebuffers() {
		VkFramebufferCreateInfo fb_ci =
			vkbuild::framebuffer_ci(mRenderPass, mWindowExtent);
		const u32 image_count = mSwapchainImages.size();
		mFramebuffers = std::vector<VkFramebuffer>(image_count);
		for (int i = 0; i < image_count; ++i) {
            VkImageView attachments[2] {mSwapchainImageViews[i], mDepthAttachment.view};
			fb_ci.pAttachments = &attachments[0];
            fb_ci.attachmentCount = 2;
			VK_CHECK(vkCreateFramebuffer(mDevice, &fb_ci, nullptr,
										 &mFramebuffers[i]));
		}
	}

	void VulkanRenderer::init_default_renderpass() {

		VkAttachmentDescription color_attachment {};
		color_attachment.format = mSwapchainImageFormat;
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

		VkAttachmentDescription depth_attachment {};
		depth_attachment.format = mDepthFormat;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attach_ref {};
        depth_attach_ref.attachment = 1;
        depth_attach_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attach_ref;

		VkSubpassDependency color_dependency = {};
		color_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		color_dependency.dstSubpass = 0;
		color_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.srcAccessMask = 0;
		color_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency depth_dependency = {};
		depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depth_dependency.dstSubpass = 0;
		depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.srcAccessMask = 0;
		depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription attachments[2] {color_attachment, depth_attachment};
        VkSubpassDependency dependencies[2] {color_dependency, depth_dependency};

		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_info.attachmentCount = 2;
		render_pass_info.pAttachments = &attachments[0];
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;
		render_pass_info.dependencyCount = 2;
		render_pass_info.pDependencies = &dependencies[0];
		VK_CHECK(vkCreateRenderPass(mDevice, &render_pass_info, nullptr,
									&mRenderPass));
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
			vkCreateFence(mDevice, &fenceCreateInfo, nullptr, &mRenderFence));

		VkSemaphoreCreateInfo semaphoreCreateInfo = vkbuild::semaphore_ci();

		VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,
								   &mPresentSemaphore));
		VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,
								   &mRenderSemaphore));
	}

	void VulkanRenderer::init_pipeline() {
		vkbuild::PipelineBuilder builder;
		mGraphicsPipelineLayout =
			builder
				.add_shader(mDevice,
							"assets/shaders/default_shader.vert.glsl.spv",
							vkbuild::ShaderType::VERTEX)
				.add_shader(mDevice,
							"assets/shaders/default_shader.frag.glsl.spv",
							vkbuild::ShaderType::FRAGMENT)
				.set_vertex_input_description(Vertex::get_description())
				.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false)
				.set_polygon_mode(VK_POLYGON_MODE_FILL)
				// @TODO: cull mode
				.set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE)
				.set_multisampling_enabled(false)
				.add_default_color_blend_attachment()
				.set_color_blending_enabled(false)
				.add_push_constant(sizeof(MeshPushConstant),
								   VK_SHADER_STAGE_VERTEX_BIT)
                .set_depth_testing(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
				/* .add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT) */
				/* .add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR) */
				/* .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH) */
				.add_viewport({0, 0, static_cast<float>(mWindowExtent.width),
							   static_cast<float>(mWindowExtent.height), 0.f,
							   1.f})
				.add_scissor({{0, 0}, mWindowExtent})
				.build_layout(mDevice);
		mGraphicsPipeline = builder.build_pipeline(mDevice, mRenderPass);
	}

	void VulkanRenderer::load_mesh() {
		ArrayList<Vertex> vertices{3};
		vertices.push_back({{1.f, 1.f, 0.f}, {1.f, 0.f, 1.f}});
		vertices.push_back({{-1.f, 1.f, 0.f}, {1.f, 0.f, 1.f}});
		vertices.push_back({{1.f, -1.f, 0.f}, {1.f, 0.f, 1.f}});
		mMesh.set_vertices(vertices).upload_mesh(mAllocator);
		mMonkeyMesh.load_from_obj("assets/models/monkey.obj");
		mMonkeyMesh.upload_mesh(mAllocator);
	}

} // namespace render
