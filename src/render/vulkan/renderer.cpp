#include "render/vulkan/renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iterator>
#include <vulkan/vulkan_core.h>
#include "../../vendor/vk-bootstrap/src/VkBootstrap.h"
#include "core/logger.h"
#include "render/vulkan/vulkan_builders.h"
#include "render/vulkan/vulkan_types.h"

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
		init_descriptors();
		init_scene();
	}

	VulkanRenderer::~VulkanRenderer() {
		// this stuff is sort of unsafe, so need to check for handles > 0 before
		// destroying, otherwise will segfault
		if (mDevice) {
			vkDeviceWaitIdle(mDevice);
			for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
				if (mFrames[i].command_pool) {
					vkDestroyCommandPool(mDevice, mFrames[i].command_pool,
										 nullptr);
				}
				if (mFrames[i].render_fence) {
					vkDestroyFence(mDevice, mFrames[i].render_fence, nullptr);
				}
				if (mFrames[i].present_semaphore) {
					vkDestroySemaphore(mDevice, mFrames[i].present_semaphore,
									   nullptr);
				}
				if (mFrames[i].render_semaphore) {
					vkDestroySemaphore(mDevice, mFrames[i].render_semaphore,
									   nullptr);
				}
				if (mFrames[i].camera_buffer.handle) {
					mFrames[i].camera_buffer.destroy();
				}
				if (mFrames[i].object_buffer.handle) {
					mFrames[i].object_buffer.destroy();
				}
			}
			if (mUploadContext.upload_fence) {
				vkDestroyFence(mDevice, mUploadContext.upload_fence, nullptr);
			}
			if (mUploadContext.command_pool) {
				vkDestroyCommandPool(mDevice, mUploadContext.command_pool,
									 nullptr);
			}
			if (mSwapchain) {
				vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
			}
			if (mRenderPass) {
				vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
			}
			for (int i = 0; i < mFramebuffers.size(); ++i) {
				if (mFramebuffers[i])
					vkDestroyFramebuffer(mDevice, mFramebuffers[i], nullptr);
				if (mSwapchainImageViews[i])
					vkDestroyImageView(mDevice, mSwapchainImageViews[i],
									   nullptr);
			}
			for (std::pair<const Material, ArrayList<Mesh>>& entry :
				 mMaterialMap) {
				vkDestroyPipelineLayout(mDevice, entry.first.layout, nullptr);
				vkDestroyPipeline(mDevice, entry.first.pipeline, nullptr);
				for (Mesh& mesh : entry.second) {
					mesh.deinit(mAllocator);
				}
			}
			mDepthAttachment.destroy();
			vkDestroyDescriptorSetLayout(mDevice, mGlobalDescriptorSetLayout,
										 nullptr);
			mScene.destroy();
			vkDestroyDescriptorSetLayout(mDevice, mObjectsDescriptorSetLayout,
										 nullptr);
			vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
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
		FrameData& frame_data = get_current_frame();
		u32 frame_index = mCurrFrame % MAXIMUM_FRAMES_IN_FLIGHT;
		VK_CHECK(vkWaitForFences(mDevice, 1, &frame_data.render_fence, true,
								 1000000000));
		VK_CHECK(vkResetFences(mDevice, 1, &frame_data.render_fence));
		// now can reset command buffer safely
		VK_CHECK(vkResetCommandBuffer(frame_data.command_buffer, 0));
		u32 image_index{};
		VK_CHECK(vkAcquireNextImageKHR(mDevice, mSwapchain, 1000000000,
									   frame_data.present_semaphore, nullptr,
									   &image_index));
		VkCommandBuffer buf = frame_data.command_buffer;
		VkCommandBufferBeginInfo buf_begin_info =
			vkbuild::command_buffer_begin_info(
				VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(buf, &buf_begin_info));
		VkClearValue color_clear{};
		VkClearValue depth_clear{};
		depth_clear.depthStencil.depth = 1.f;

		VkRenderPassBeginInfo renderpass_info = vkbuild::renderpass_begin_info(
			mRenderPass, mWindowExtent, mFramebuffers[image_index]);
		float flash = abs(sin(mCurrFrame / 120.f));
		color_clear.color = {{0.f, 0.f, flash, 1.f}};
		VkClearValue clear_values[]{color_clear, depth_clear};
		renderpass_info.clearValueCount = 2;
		renderpass_info.pClearValues = &clear_values[0];
		vkCmdBeginRenderPass(buf, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

		// insert actual commands
		VkDeviceSize offset{0};
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
		glm::mat4 model =
			glm::rotate(glm::mat4{1.0f}, glm::radians(mCurrFrame * 0.4f),
						glm::vec3(0, 1, 0));
		// calculate final mesh matrix
		MeshPushConstant constants;
		constants.render_matrix = model;
		CameraData cam_data = {};
		cam_data.proj = projection;
		cam_data.view = view;
		cam_data.view_proj = projection * view;
		void* data; // classic approach
		vmaMapMemory(mAllocator, frame_data.camera_buffer.memory, &data);
		memcpy(data, &cam_data, sizeof(CameraData));
		vmaUnmapMemory(mAllocator, frame_data.camera_buffer.memory);
		void* object_data; // lil trick
		vmaMapMemory(mAllocator, frame_data.object_buffer.memory, &object_data);
		ObjectData* object_ssbo = static_cast<ObjectData*>(object_data);
		int ssbo_index{0};
		for (std::pair<const Material, ArrayList<Mesh>>& entry : mMaterialMap) {
			for (Mesh& mesh : entry.second) {
				mesh.transform = model;
				object_ssbo[ssbo_index].model_matrix = mesh.transform;
				ssbo_index++;
			}
		}
		vmaUnmapMemory(mAllocator, frame_data.object_buffer.memory);
		u32 uniform_offset =
			pad_uniform_buffer(sizeof(SceneData) * frame_index);
		mScene.write_to_buffer(uniform_offset);
		for (std::pair<const Material, ArrayList<Mesh>>& entry : mMaterialMap) {
			vkCmdBindPipeline(buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
							  entry.first.pipeline);
			vkCmdBindDescriptorSets(
				buf, VK_PIPELINE_BIND_POINT_GRAPHICS, entry.first.layout, 0, 1,
				&frame_data.global_descriptor, 1, &uniform_offset);
			vkCmdBindDescriptorSets(buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
									entry.first.layout, 1, 1,
									&frame_data.object_descriptor, 0, nullptr);
			if(entry.first.textureSet != VK_NULL_HANDLE){
				vkCmdBindDescriptorSets(buf, VK_PIPELINE_BIND_POINT_GRAPHICS,  entry.first.layout, 2, 1, &entry.first.textureSet, 0, nullptr);
			}
			for (Mesh& mesh : entry.second) {
				vkCmdBindVertexBuffers(buf, 0, 1, &mesh.buffer.handle, &offset);
				// upload the matrix to the GPU via push constants
				vkCmdPushConstants(buf, entry.first.layout,
								   VK_SHADER_STAGE_VERTEX_BIT, 0,
								   sizeof(MeshPushConstant), &constants);
				vkCmdDraw(buf, mesh.vertices.size(), 1, 0, 0);
			}
		}

		vkCmdEndRenderPass(buf);
		VK_CHECK(vkEndCommandBuffer(buf));
		// waiting on present_semaphore which is signaled when swapchain is
		// ready. signal render_semaphore when finished rendering.
		VkSubmitInfo submit = vkbuild::submit_info(&buf);
		VkPipelineStageFlags wait_stage =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit.pWaitDstStageMask = &wait_stage;
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &frame_data.present_semaphore;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &frame_data.render_semaphore;
		// render fence blocks until commands finish executing
		VK_CHECK(
			vkQueueSubmit(mGraphicsQueue, 1, &submit, frame_data.render_fence));

		// waiting on rendering to finish, then presenting
		VkPresentInfoKHR present_info = vkbuild::present_info();
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &mSwapchain;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &frame_data.render_semaphore;
		present_info.pImageIndices = &image_index;
		VK_CHECK(vkQueuePresentKHR(mGraphicsQueue, &present_info));
		++mCurrFrame;
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
		VkPhysicalDeviceShaderDrawParametersFeatures shader_dram_param_feat{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
			nullptr, VK_TRUE};
		vkb::DeviceBuilder dev_builder{vkb_phys_dev};
		vkb::Device vkb_dev =
			dev_builder.add_pNext(&shader_dram_param_feat).build().value();
		mDevice = vkb_dev.device;
		mPhysicalDevice = vkb_phys_dev.physical_device;
		mPhysicalDeviceProperties = vkb_dev.physical_device.properties;
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
		VmaAllocationCreateInfo imgAllocCi{};
		imgAllocCi.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		imgAllocCi.requiredFlags =
			VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		auto res = Image::create_image(
			mAllocator, mDevice,
			vkbuild::image_ci(mDepthFormat,
							  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
							  depthImageExtent),
			imgAllocCi);
		if (res.has_value()) {
			bool result = res->create_view(vkbuild::imageview_ci(
				mDepthFormat, res->handle, VK_IMAGE_ASPECT_DEPTH_BIT));
			if (!result) {
				return;
			}
			mDepthAttachment = std::move(res.value());
		}
	}

	void VulkanRenderer::init_commands() {
		for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			VkCommandPoolCreateInfo command_pool_ci = vkbuild::command_pool_ci(
				mGraphicsQueueFamily,
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			VK_CHECK(vkCreateCommandPool(mDevice, &command_pool_ci, nullptr,
										 &mFrames[i].command_pool));
			VkCommandBufferAllocateInfo alloc_info =
				vkbuild::command_buffer_ai(mFrames[i].command_pool, 1);
			VK_CHECK(vkAllocateCommandBuffers(mDevice, &alloc_info,
											  &mFrames[i].command_buffer));
		}
		VkCommandPoolCreateInfo upload_ctx_cmd_pool_ci{
			vkbuild::command_pool_ci(mGraphicsQueueFamily)};
		VK_CHECK(vkCreateCommandPool(mDevice, &upload_ctx_cmd_pool_ci, nullptr,
									 &mUploadContext.command_pool));
		VkCommandBufferAllocateInfo upload_ctx_cmd_ai{
			vkbuild::command_buffer_ai(mUploadContext.command_pool)};
		VK_CHECK(vkAllocateCommandBuffers(mDevice, &upload_ctx_cmd_ai,
										  &mUploadContext.command_buffer));
	}

	void VulkanRenderer::init_framebuffers() {
		VkFramebufferCreateInfo fb_ci =
			vkbuild::framebuffer_ci(mRenderPass, mWindowExtent);
		const u32 image_count = mSwapchainImages.size();
		mFramebuffers = std::vector<VkFramebuffer>(image_count);
		for (int i = 0; i < image_count; ++i) {
			VkImageView attachments[2]{mSwapchainImageViews[i],
									   mDepthAttachment.view};
			fb_ci.pAttachments = &attachments[0];
			fb_ci.attachmentCount = 2;
			VK_CHECK(vkCreateFramebuffer(mDevice, &fb_ci, nullptr,
										 &mFramebuffers[i]));
		}
	}

	void VulkanRenderer::init_default_renderpass() {

		VkAttachmentDescription color_attachment{};
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

		VkAttachmentDescription depth_attachment{};
		depth_attachment.format = mDepthFormat;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attach_ref{};
		depth_attach_ref.attachment = 1;
		depth_attach_ref.layout =
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attach_ref;

		VkSubpassDependency color_dependency = {};
		color_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		color_dependency.dstSubpass = 0;
		color_dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.srcAccessMask = 0;
		color_dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		color_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency depth_dependency = {};
		depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depth_dependency.dstSubpass = 0;
		depth_dependency.srcStageMask =
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.srcAccessMask = 0;
		depth_dependency.dstStageMask =
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depth_dependency.dstAccessMask =
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkAttachmentDescription attachments[2]{color_attachment,
											   depth_attachment};
		VkSubpassDependency dependencies[2]{color_dependency, depth_dependency};

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
		for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			VkFenceCreateInfo fenceCreateInfo =
				vkbuild::fence_ci(VK_FENCE_CREATE_SIGNALED_BIT);
			VK_CHECK(vkCreateFence(mDevice, &fenceCreateInfo, nullptr,
								   &mFrames[i].render_fence));
			VkSemaphoreCreateInfo semaphoreCreateInfo = vkbuild::semaphore_ci();
			VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,
									   &mFrames[i].present_semaphore));
			VK_CHECK(vkCreateSemaphore(mDevice, &semaphoreCreateInfo, nullptr,
									   &mFrames[i].render_semaphore));
		}
		// no signaled bit because we don't have to wait
		VkFenceCreateInfo upload_fence_ci{vkbuild::fence_ci()};
		VK_CHECK(vkCreateFence(mDevice, &upload_fence_ci, nullptr,
							   &mUploadContext.upload_fence));
	}

	void VulkanRenderer::init_scene() {
		{
			Material material{};
			vkbuild::PipelineBuilder builder;
			material.layout =
				builder
					.add_shader(mDevice,
								"assets/shaders/default_shader.vert.glsl.spv",
								vkbuild::ShaderType::VERTEX)
					.add_shader(mDevice,
								"assets/shaders/default_shader.frag.glsl.spv",
								vkbuild::ShaderType::FRAGMENT)
					.set_vertex_input_description(Vertex::get_description())
					.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
										false)
					.set_polygon_mode(VK_POLYGON_MODE_FILL)
					// @TODO: cull mode
					.set_cull_mode(VK_CULL_MODE_BACK_BIT,
								   VK_FRONT_FACE_COUNTER_CLOCKWISE)
					.set_multisampling_enabled(false)
					.add_default_color_blend_attachment()
					.set_color_blending_enabled(false)
					.add_push_constant(sizeof(MeshPushConstant),
									   VK_SHADER_STAGE_VERTEX_BIT)
					.add_descriptor_set_layout(mGlobalDescriptorSetLayout)
					.add_descriptor_set_layout(mObjectsDescriptorSetLayout)
					.set_depth_testing(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
					/* .add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT) */
					/* .add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR) */
					/* .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH) */
					.add_viewport(
						{0, 0, static_cast<float>(mWindowExtent.width),
						 static_cast<float>(mWindowExtent.height), 0.f, 1.f})
					.add_scissor({{0, 0}, mWindowExtent})
					.build_layout(mDevice);
			material.pipeline = builder.build_pipeline(mDevice, mRenderPass);
			Mesh monkeyMesh{};
			monkeyMesh.load_from_obj("assets/models/monkey.obj");
			upload_mesh(monkeyMesh);
			add_material_to_mesh(material, monkeyMesh);
		}
		{
			Material material{};
			VkSamplerCreateInfo sampler_info =
				vkbuild::sampler_create_info(VK_FILTER_NEAREST);
			VkSampler sampler;
			vkCreateSampler(mDevice, &sampler_info, nullptr, &sampler);
			VkDescriptorSetAllocateInfo texture_alloc_info{
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
				mDescriptorPool, 1, &mTextureSamplerDescriptorSetLayout};
			VK_CHECK(vkAllocateDescriptorSets(mDevice, &texture_alloc_info, &material.textureSet));
			Image texture {"assets/textures/lost_empire-RGBA.png", mAllocator, mDevice, *this};
			VkDescriptorImageInfo image_buf_info {sampler, texture.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
			VkWriteDescriptorSet tex_write = vkbuild::write_descriptor_image(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, material.textureSet, &image_buf_info, 0);
			vkUpdateDescriptorSets(mDevice, 1, &tex_write, 0, nullptr);
			vkbuild::PipelineBuilder builder;
			material.layout =
				builder
					.add_shader(mDevice,
								"assets/shaders/textured_mesh.vert.glsl.spv",
								vkbuild::ShaderType::VERTEX)
					.add_shader(mDevice,
								"assets/shaders/textured_mesh.frag.glsl.spv",
								vkbuild::ShaderType::FRAGMENT)
					.set_vertex_input_description(Vertex::get_description())
					.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
										false)
					.set_polygon_mode(VK_POLYGON_MODE_FILL)
					// @TODO: cull mode
					.set_cull_mode(VK_CULL_MODE_BACK_BIT,
								   VK_FRONT_FACE_COUNTER_CLOCKWISE)
					.set_multisampling_enabled(false)
					.add_default_color_blend_attachment()
					.set_color_blending_enabled(false)
					.add_push_constant(sizeof(MeshPushConstant),
									   VK_SHADER_STAGE_VERTEX_BIT)
					.add_descriptor_set_layout(mGlobalDescriptorSetLayout)
					.add_descriptor_set_layout(mObjectsDescriptorSetLayout)
					.add_descriptor_set_layout(
						mTextureSamplerDescriptorSetLayout)
					.set_depth_testing(true, true, VK_COMPARE_OP_LESS_OR_EQUAL)
					/* .add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT) */
					/* .add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR) */
					/* .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH) */
					.add_viewport(
						{0, 0, static_cast<float>(mWindowExtent.width),
						 static_cast<float>(mWindowExtent.height), 0.f, 1.f})
					.add_scissor({{0, 0}, mWindowExtent})
					.build_layout(mDevice);
			material.pipeline = builder.build_pipeline(mDevice, mRenderPass);
			Mesh lost_empire{};
			lost_empire.load_from_obj("assets/models/lost_empire.obj");
			upload_mesh(lost_empire);
			add_material_to_mesh(material, lost_empire);
		}
		mScene.scene_data.ambient_color = {0.7f, 0.4f, 0.1f, 0.f};
	}

	void VulkanRenderer::init_descriptors() {
		const size_t scene_param_buffer_size =
			MAXIMUM_FRAMES_IN_FLIGHT * pad_uniform_buffer(sizeof(SceneData));
		mScene = {mAllocator, scene_param_buffer_size, {}};
		VkDescriptorPoolSize sizes[]{
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10}};
		VkDescriptorPoolCreateInfo pool_ci{
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			nullptr,
			0,
			10,
			static_cast<u32>(std::size(sizes)),
			&sizes[0]};
		VK_CHECK(vkCreateDescriptorPool(mDevice, &pool_ci, nullptr,
										&mDescriptorPool));
		VkDescriptorSetLayoutBinding cam_buffer_binding =
			vkbuild::descriptorset_layout_binding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,
				0);
		VkDescriptorSetLayoutBinding scene_buffer_binding =
			vkbuild::descriptorset_layout_binding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1);
		VkDescriptorSetLayoutBinding bindings[]{cam_buffer_binding,
												scene_buffer_binding};
		VkDescriptorSetLayoutBinding object_buffer_binding =
			vkbuild::descriptorset_layout_binding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT,
				0);
		VkDescriptorSetLayoutBinding texture_sampler_binding =
			vkbuild::descriptorset_layout_binding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT, 0);
		VkDescriptorSetLayoutCreateInfo scene_set_ci{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0,
			std::size(bindings), bindings};
		VkDescriptorSetLayoutCreateInfo object_set_ci{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1,
			&object_buffer_binding};
		VkDescriptorSetLayoutCreateInfo sampler_descriptor_set{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, 1,
			&texture_sampler_binding};
		VK_CHECK(vkCreateDescriptorSetLayout(mDevice, &scene_set_ci, nullptr,
											 &mGlobalDescriptorSetLayout));
		VK_CHECK(vkCreateDescriptorSetLayout(mDevice, &object_set_ci, nullptr,
											 &mObjectsDescriptorSetLayout));
		VK_CHECK(vkCreateDescriptorSetLayout(
			mDevice, &sampler_descriptor_set, nullptr,
			&mTextureSamplerDescriptorSetLayout));
		for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			mFrames[i].camera_buffer = {mAllocator, sizeof(CameraData),
										VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
										VMA_MEMORY_USAGE_CPU_TO_GPU};
			mFrames[i].object_buffer = {mAllocator,
										sizeof(ObjectData) * MAX_OBJECTS,
										VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
										VMA_MEMORY_USAGE_CPU_TO_GPU};
			VkDescriptorSetAllocateInfo scene_buffer_alloc_info{
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
				mDescriptorPool, 1, &mGlobalDescriptorSetLayout};
			VK_CHECK(vkAllocateDescriptorSets(mDevice, &scene_buffer_alloc_info,
											  &mFrames[i].global_descriptor));
			VkDescriptorSetAllocateInfo objects_buffer_alloc_info{
				VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr,
				mDescriptorPool, 1, &mObjectsDescriptorSetLayout};
			VK_CHECK(vkAllocateDescriptorSets(mDevice,
											  &objects_buffer_alloc_info,
											  &mFrames[i].object_descriptor));

			VK_CHECK(vkAllocateDescriptorSets(mDevice,
											  &objects_buffer_alloc_info,
											  &mFrames[i].object_descriptor));
			VkDescriptorBufferInfo camera_buffer_info{
				mFrames[i].camera_buffer.handle, 0, sizeof(CameraData)};
			VkDescriptorBufferInfo scene_buffer_info = mScene.buffer_info(0);
			VkDescriptorBufferInfo object_buffer_info{
				mFrames[i].object_buffer.handle, 0,
				sizeof(ObjectData) * MAX_OBJECTS};
			VkWriteDescriptorSet camera_write =
				vkbuild::write_descriptor_buffer(
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					mFrames[i].global_descriptor, &camera_buffer_info, 0);
			VkWriteDescriptorSet scene_write = vkbuild::write_descriptor_buffer(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				mFrames[i].global_descriptor, &scene_buffer_info, 1);
			VkWriteDescriptorSet objects_write =
				vkbuild::write_descriptor_buffer(
					VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
					mFrames[i].object_descriptor, &object_buffer_info, 0);
			VkWriteDescriptorSet set_writes[]{camera_write, scene_write,
											  objects_write};
			vkUpdateDescriptorSets(mDevice, 3, set_writes, 0, nullptr);
		}
	}

	void VulkanRenderer::add_material_to_mesh(const Material& material,
											  const Mesh& mesh) {
		if (mMaterialMap.contains(material)) {
			mMaterialMap[material].push_back(mesh);
			return;
		}
		mMaterialMap.insert({material, {mesh}});
	}

	size_t VulkanRenderer::pad_uniform_buffer(size_t original_size) {
		// Calculate required alignment based on minimum device offset alignment
		size_t minUboAlignment =
			mPhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
		size_t alignedSize = original_size;
		if (minUboAlignment > 0) {
			alignedSize =
				(alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		return alignedSize;
	}

	void VulkanRenderer::upload_mesh(Mesh& mesh) {
		const size_t buf_size = mesh.vertices.size() * sizeof(Vertex);
		Buffer staging{mAllocator, buf_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					   VMA_MEMORY_USAGE_CPU_ONLY};
		void* data;
		vmaMapMemory(mAllocator, staging.memory, &data);
		memcpy(data, mesh.vertices.data(), buf_size);
		vmaUnmapMemory(mAllocator, staging.memory);
		mesh.buffer = {mAllocator, buf_size,
					   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
						   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					   VMA_MEMORY_USAGE_GPU_ONLY};
		immediate_submit([=](VkCommandBuffer cmd) {
			VkBufferCopy copy{0, 0, buf_size};
			vkCmdCopyBuffer(cmd, staging.handle, mesh.buffer.handle, 1, &copy);
		});
		staging.destroy();
	}

	void VulkanRenderer::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& fn) {
		VkCommandBuffer cmd = mUploadContext.command_buffer;
		VkCommandBufferBeginInfo begin_info{vkbuild::command_buffer_begin_info(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)};
		VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));
		fn(cmd);
		VK_CHECK(vkEndCommandBuffer(cmd));
		VkSubmitInfo submit{vkbuild::submit_info(&cmd)};
		VK_CHECK(vkQueueSubmit(mGraphicsQueue, 1, &submit,
							   mUploadContext.upload_fence));
		vkWaitForFences(mDevice, 1, &mUploadContext.upload_fence, true,
						1000000000);
		vkResetFences(mDevice, 1, &mUploadContext.upload_fence);
		vkResetCommandPool(mDevice, mUploadContext.command_pool, 0);
	}
} // namespace render
