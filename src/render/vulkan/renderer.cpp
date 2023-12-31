#include "render/vulkan/renderer.h"
#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_video.h>
#include <SDL_vulkan.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iterator>
#include <ranges>
#include <vulkan/vulkan_core.h>
#include "../../vendor/vk-bootstrap/src/VkBootstrap.h"
#include "core/input.h"
#include "core/logger.h"
#include "render/vulkan/builders.h"
#include "render/vulkan/mesh.h"
#include "render/vulkan/pipeline.h"
#include "render/vulkan/types.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace render::vulkan {
	VulkanRenderer::VulkanRenderer() {
		SDL_Init(SDL_INIT_VIDEO);
		mpWindow = SDL_CreateWindow(
			"Guccigedon", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			mWindowExtent.width, mWindowExtent.height,
			(SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE));
		if (!mpWindow) {
			core::Logger::Fatal("Failed to create a window. {}",
								SDL_GetError());
			exit(-1);
		}
		init_instance();
		init_swapchain();
		init_default_renderpass();
		init_framebuffers();
		init_commands();
		init_sync_objects();
		init_descriptors();
		/* init_scene(); */
		mCamera.transform.position({0.f, 0.f, 2.f});
		for (std::pair<const Material, ArrayList<Mesh>>& entry : mMaterialMap) {
			mMaterialBufferMap[entry.first] = merge_vertices(entry.second);
		}
		mCamera = {glm::radians(70.f),
				   static_cast<f32>(mWindowExtent.width) / mWindowExtent.height,
				   0.1f, 200.0f};
		mCamera.transform.position({0, 0, 3});
		mImageCache = {mDevice, this};
		mShaderCache = {mDevice};
	}

	VulkanRenderer ::VulkanRenderer(std::filesystem::path scene_path) {
		SDL_Init(SDL_INIT_VIDEO);
		mpWindow = SDL_CreateWindow(
			"Guccigedon", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			mWindowExtent.width, mWindowExtent.height,
			(SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE));
		if (!mpWindow) {
			core::Logger::Fatal("Failed to create a window. {}",
								SDL_GetError());
			exit(-1);
		}
		init_instance();
		init_swapchain();
		init_default_renderpass();
		init_framebuffers();
		init_commands();
		init_sync_objects();
		init_descriptors();
		/* init_scene(); */
		mCamera.transform.position({0.f, 0.f, 2.f});
		for (std::pair<const Material, ArrayList<Mesh>>& entry : mMaterialMap) {
			mMaterialBufferMap[entry.first] = merge_vertices(entry.second);
		}
		mCamera = {glm::radians(70.f),
				   static_cast<f32>(mWindowExtent.width) / mWindowExtent.height,
				   0.1f, 200.0f};
		mCamera.transform.position({0, 0, 3});
		mImageCache = {mDevice, this};
		mShaderCache = {mDevice};
		mGltfScene = {scene_path, &mDevice, this};
	}

    VulkanRenderer::VulkanRenderer(const asset::GLTFImporter& scene_asset){
		SDL_Init(SDL_INIT_VIDEO);
		mpWindow = SDL_CreateWindow(
			"Guccigedon", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			mWindowExtent.width, mWindowExtent.height,
			(SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE));
		if (!mpWindow) {
			core::Logger::Fatal("Failed to create a window. {}",
								SDL_GetError());
			exit(-1);
		}
		init_instance();
		init_swapchain();
		init_default_renderpass();
		init_framebuffers();
		init_commands();
		init_sync_objects();
		init_descriptors();
		/* init_scene(); */
		mCamera.transform.position({0.f, 0.f, 2.f});
		for (std::pair<const Material, ArrayList<Mesh>>& entry : mMaterialMap) {
			mMaterialBufferMap[entry.first] = merge_vertices(entry.second);
		}
		mCamera = {glm::radians(70.f),
				   static_cast<f32>(mWindowExtent.width) / mWindowExtent.height,
				   0.1f, 200.0f};
		mCamera.transform.position({0, 0, 3});
		mImageCache = {mDevice, this};
		mShaderCache = {mDevice};
        mGltfScene = {scene_asset, &mDevice, this};
    }

	VulkanRenderer::VulkanRenderer(VulkanRenderer&& other) noexcept {
		mpWindow = other.mpWindow;
		mWindowExtent = other.mWindowExtent;
		mInstance = std::move(other.mInstance);
		mDevice = std::move(other.mDevice);
		mSurface = std::move(other.mSurface);
		mSwapchain = std::move(other.mSwapchain);
		mRenderPass = std::move(other.mRenderPass);
		mMaterialMap = std::move(other.mMaterialMap);
		mMaterialBufferMap = std::move(other.mMaterialBufferMap);
		for (auto i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			mFrames[i] = std::move(other.mFrames[i]);
		}
		mDescriptorAllocatorPool = std::move(other.mDescriptorAllocatorPool);
		mMainDescriptorAllocator = std::move(other.mMainDescriptorAllocator);
		mDescriptorLayoutCache = std::move(other.mDescriptorLayoutCache);
		mGlobalDescriptorSetLayout =
			std::move(other.mGlobalDescriptorSetLayout);
		mObjectsDescriptorSetLayout =
			std::move(other.mObjectsDescriptorSetLayout);
		mTextureSamplerDescriptorSetLayout =
			std::move(other.mTextureSamplerDescriptorSetLayout);
		mScene = std::move(other.mScene);
		mGltfScene = std::move(other.mGltfScene);
		mUploadContext = std::move(other.mUploadContext);
		mCurrFrame = other.mCurrFrame;
		mShouldResize = other.mShouldResize;
		mShaderCache = std::move(other.mShaderCache);
		mCamera = std::move(other.mCamera);
		mImageCache = std::move(other.mImageCache);
		other.mpWindow = nullptr;
		other.mGlobalDescriptorSetLayout = nullptr;
		other.mObjectsDescriptorSetLayout = nullptr;
		other.mTextureSamplerDescriptorSetLayout = nullptr;
	}

	VulkanRenderer& VulkanRenderer::operator=(VulkanRenderer&& other) noexcept {
		mpWindow = other.mpWindow;
		mWindowExtent = other.mWindowExtent;
		mInstance = std::move(other.mInstance);
		mDevice = std::move(other.mDevice);
		mSurface = std::move(other.mSurface);
		mSwapchain = std::move(other.mSwapchain);
		mRenderPass = std::move(other.mRenderPass);
		mMaterialMap = std::move(other.mMaterialMap);
		mMaterialBufferMap = std::move(other.mMaterialBufferMap);
		for (auto i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			mFrames[i] = std::move(other.mFrames[i]);
		}
		mDescriptorAllocatorPool = std::move(other.mDescriptorAllocatorPool);
		mMainDescriptorAllocator = std::move(other.mMainDescriptorAllocator);
		mDescriptorLayoutCache = std::move(other.mDescriptorLayoutCache);
		mGlobalDescriptorSetLayout =
			std::move(other.mGlobalDescriptorSetLayout);
		mObjectsDescriptorSetLayout =
			std::move(other.mObjectsDescriptorSetLayout);
		mTextureSamplerDescriptorSetLayout =
			std::move(other.mTextureSamplerDescriptorSetLayout);
		mScene = std::move(other.mScene);
		mGltfScene = std::move(other.mGltfScene);
		mUploadContext = std::move(other.mUploadContext);
		mCurrFrame = other.mCurrFrame;
		mShouldResize = other.mShouldResize;
		mShaderCache = std::move(other.mShaderCache);
		mCamera = std::move(other.mCamera);
		mImageCache = std::move(other.mImageCache);
		other.mpWindow = nullptr;
		other.mGlobalDescriptorSetLayout = nullptr;
		other.mObjectsDescriptorSetLayout = nullptr;
		other.mTextureSamplerDescriptorSetLayout = nullptr;
		return *this;
	}

	VulkanRenderer::~VulkanRenderer() {
		// this stuff is sort of unsafe, so need to check for handles > 0 before
		// destroying, otherwise will segfault
		mDevice.wait_idle();
		for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			if (mFrames[i].command_pool) {
				vkDestroyCommandPool(mDevice.logical_device(),
									 mFrames[i].command_pool, nullptr);
			}
			if (mFrames[i].render_fence) {
				vkDestroyFence(mDevice.logical_device(),
							   mFrames[i].render_fence, nullptr);
			}
			if (mFrames[i].present_semaphore) {
				vkDestroySemaphore(mDevice.logical_device(),
								   mFrames[i].present_semaphore, nullptr);
			}
			if (mFrames[i].render_semaphore) {
				vkDestroySemaphore(mDevice.logical_device(),
								   mFrames[i].render_semaphore, nullptr);
			}
			if (mFrames[i].camera_buffer.handle) {
				mFrames[i].camera_buffer.destroy();
			}
			if (mFrames[i].object_buffer.handle) {
				mFrames[i].object_buffer.destroy();
			}
		}
		if (mUploadContext.upload_fence) {
			vkDestroyFence(mDevice.logical_device(),
						   mUploadContext.upload_fence, nullptr);
		}
		if (mUploadContext.command_pool) {
			vkDestroyCommandPool(mDevice.logical_device(),
								 mUploadContext.command_pool, nullptr);
		}
		if (mRenderPass) {
			vkDestroyRenderPass(mDevice.logical_device(), mRenderPass, nullptr);
		}
		for (std::pair<const Material, ArrayList<Mesh>>& entry : mMaterialMap) {
			vkDestroyPipelineLayout(mDevice.logical_device(),
									entry.first.layout, nullptr);
			vkDestroyPipeline(mDevice.logical_device(), entry.first.pipeline,
							  nullptr);
			Buffer& vb = mMaterialBufferMap[entry.first].buffer;
			vb.destroy();
			for (Mesh& mesh : entry.second) {
				mesh.deinit(mDevice.allocator());
			}
		}
		mScene.destroy();
		if (mpWindow) {
			SDL_DestroyWindow(mpWindow);
		}
	}

	void VulkanRenderer::draw(const ArrayList<gameplay::Transform>& transforms) {
		// Not rendering when minimized
		if (SDL_GetWindowFlags(mpWindow) & SDL_WINDOW_MINIMIZED) {
			return;
		}
		FrameData& frame_data = get_current_frame();
		u32 frame_index = mCurrFrame % MAXIMUM_FRAMES_IN_FLIGHT;
		u32 image_index{};
		begin_renderpass(frame_data, image_index);
		VkCommandBuffer& buf = frame_data.command_buffer;
		// insert actual commands
		VkDeviceSize offset{0};
		// model rotation
		// calculate final mesh matrix
		MeshPushConstant constants;
		// make a model view matrix for rendering the object
		CameraData cam_data = {};
		cam_data.proj = mCamera.projection;
		cam_data.view = mCamera.view();
		cam_data.view_proj = mCamera.view_proj();
		void* data; // classic approach
		vmaMapMemory(mDevice.allocator(), frame_data.camera_buffer.memory,
					 &data);
		memcpy(data, &cam_data, sizeof(CameraData));
		vmaUnmapMemory(mDevice.allocator(), frame_data.camera_buffer.memory);
		void* object_data; // lil trick
		vmaMapMemory(mDevice.allocator(), frame_data.object_buffer.memory,
					 &object_data);
		ObjectData* object_ssbo = static_cast<ObjectData*>(object_data);
        u32 ssbo_index{0};
        for(auto& transform : transforms){
            object_ssbo[ssbo_index].model_matrix = transform.transform();
            ssbo_index++;
        }
		vmaUnmapMemory(mDevice.allocator(), frame_data.object_buffer.memory);
		u32 uniform_offset =
			pad_uniform_buffer(sizeof(SceneData) * frame_index);
		mScene.write_to_buffer(uniform_offset);
		VkViewport vp{};
		vp.height = static_cast<f32>(mWindowExtent.height);
		vp.width = static_cast<f32>(mWindowExtent.width);
		vp.maxDepth = 1.f;
		vp.minDepth = 0.f;
		vp.x = 0.f;
		vp.y = 0.f;
		vkCmdSetViewport(buf, 0, 1, &vp);
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = mWindowExtent;
		vkCmdSetScissor(buf, 0, 1, &scissor);
		mGltfScene.draw(buf, object_ssbo, frame_data, uniform_offset);
		end_renderpass(buf);
		mDevice.submit_queue(buf, frame_data.present_semaphore,
							 frame_data.render_semaphore,
							 frame_data.render_fence,
							 VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		mDevice.present(mSwapchain.handle(), frame_data.render_semaphore,
						image_index, std::bind(&VulkanRenderer::resize, this));
		++mCurrFrame;
	}

	void VulkanRenderer::resize() {
		int w, h;
		SDL_GetWindowSize(mpWindow, &w, &h);
		core::Logger::Warning("Resizing: {}, {}", w, h);
		mWindowExtent = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
		mSwapchain.rebuild(w, h, mRenderPass);
		mShouldResize = false;
	}

	void VulkanRenderer::end_renderpass(VkCommandBuffer buf) {
		vkCmdEndRenderPass(buf);
		VK_CHECK(vkEndCommandBuffer(buf));
	}

	void VulkanRenderer::begin_renderpass(FrameData& frame_data,
										  u32& image_index) {
		// wait until last frame is rendered, timeout 1s
		VK_CHECK(vkWaitForFences(mDevice.logical_device(), 1,
								 &frame_data.render_fence, true, 1000000000));
		if (mShouldResize) {
			int w, h;
			SDL_Vulkan_GetDrawableSize(mpWindow, &w, &h);
			core::Logger::Warning("Resizing in get next imag {}, {}", w, h);
			mWindowExtent = {static_cast<uint32_t>(w),
							 static_cast<uint32_t>(h)};
			mSwapchain.rebuild(w, h, mRenderPass);
			mCamera.aspect = w / (f32)h;
			mCamera.build_projection();
			mShouldResize = false;
		}
		VK_CHECK(vkResetFences(mDevice.logical_device(), 1,
							   &frame_data.render_fence));
		// now can reset command buffer safely
		VK_CHECK(vkResetCommandBuffer(frame_data.command_buffer, 0));
		auto res = vkAcquireNextImageKHR(
			mDevice.logical_device(), mSwapchain.handle(), 1000000000,
			frame_data.present_semaphore, nullptr, &image_index);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
			int w, h;
			SDL_Vulkan_GetDrawableSize(mpWindow, &w, &h);
			core::Logger::Warning("Resizing in get next image: {}, {}", w, h);
			mWindowExtent = {static_cast<uint32_t>(w),
							 static_cast<uint32_t>(h)};
			mSwapchain.rebuild(w, h, mRenderPass);
			mCamera.aspect = w / (f32)h;
			mCamera.build_projection();
		} else if (res != VK_SUCCESS) {
			core::Logger::Error("Cannot acquire next image. {}",
								static_cast<u32>(res));
			return;
		}
		VkCommandBuffer buf = frame_data.command_buffer;
		VkCommandBufferBeginInfo buf_begin_info =
			builder::command_buffer_begin_info(
				VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VK_CHECK(vkBeginCommandBuffer(buf, &buf_begin_info));
		VkClearValue color_clear{};
		VkClearValue depth_clear{};
		depth_clear.depthStencil.depth = 1.f;
		VkRenderPassBeginInfo renderpass_info = builder::renderpass_begin_info(
			mRenderPass, mWindowExtent, mSwapchain.framebuffers()[image_index]);
		float flash = abs(sin(mCurrFrame / 1200.f));
		color_clear.color = {{0.f, 0.f, flash, 1.f}};
		VkClearValue clear_values[]{color_clear, depth_clear};
		renderpass_info.clearValueCount = 2;
		renderpass_info.pClearValues = &clear_values[0];
		vkCmdBeginRenderPass(buf, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderer::handle_input_event(core::PollResult& poll_result) {
		if (poll_result.event.type == SDL_WINDOWEVENT_RESIZED ||
			poll_result.event.type == SDL_WINDOWEVENT_SIZE_CHANGED) {
			core::Logger::Trace("Resizing set");
			mShouldResize = true;
		};
		if (core::InputSystem::mouse_state().RMB) {
			mCamera.input.process_input_event(&poll_result.event);
		} else {
			mCamera.input.reset_input_axis();
		}
		mCamera.update(1);
	}

	void VulkanRenderer::init_instance() {
		vkb::InstanceBuilder builder;
		vkb::Instance vkb_inst =
			builder.set_app_name("Guccigedon")
				.request_validation_layers(true)
				.set_debug_callback(
					[](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
					   VkDebugUtilsMessageTypeFlagsEXT messageType,
					   const VkDebugUtilsMessengerCallbackDataEXT*
						   pCallbackData,
					   void* pUserData) -> VkBool32 {
						auto severity =
							vkb::to_string_message_severity(messageSeverity);
						auto type = vkb::to_string_message_type(messageType);
						core::Logger::Error("Type: {}. Message: {}", type,
											pCallbackData->pMessage);
						return VK_FALSE;
					})
				.require_api_version(1, 1, 0)
				.build()
				.value();
		mInstance = {vkb_inst};
		mSurface = {mpWindow, mInstance.handle()};
		mDevice = {vkb_inst, mSurface.surface()};
	}

	void VulkanRenderer::init_swapchain() {
		mSwapchain = {mDevice.allocator(), mDevice, mSurface, mWindowExtent};
	}

	void VulkanRenderer::init_commands() {
		for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			VkCommandPoolCreateInfo command_pool_ci = builder::command_pool_ci(
				mDevice.graphics_queue_family(),
				VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			VK_CHECK(vkCreateCommandPool(mDevice.logical_device(),
										 &command_pool_ci, nullptr,
										 &mFrames[i].command_pool));
			VkCommandBufferAllocateInfo alloc_info =
				builder::command_buffer_ai(mFrames[i].command_pool, 1);
			VK_CHECK(vkAllocateCommandBuffers(mDevice.logical_device(),
											  &alloc_info,
											  &mFrames[i].command_buffer));
		}
		VkCommandPoolCreateInfo upload_ctx_cmd_pool_ci{
			builder::command_pool_ci(mDevice.graphics_queue_family())};
		VK_CHECK(vkCreateCommandPool(mDevice.logical_device(),
									 &upload_ctx_cmd_pool_ci, nullptr,
									 &mUploadContext.command_pool));
		VkCommandBufferAllocateInfo upload_ctx_cmd_ai{
			builder::command_buffer_ai(mUploadContext.command_pool)};
		VK_CHECK(vkAllocateCommandBuffers(mDevice.logical_device(),
										  &upload_ctx_cmd_ai,
										  &mUploadContext.command_buffer));
	}

	void VulkanRenderer::init_framebuffers() {
		mSwapchain.init_framebuffers(mRenderPass, &mWindowExtent);
	}

	void VulkanRenderer::init_default_renderpass() {

		VkAttachmentDescription color_attachment{};
		color_attachment.format = mSwapchain.image_format();
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
		depth_attachment.format = mSwapchain.depth_format();
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
		VK_CHECK(vkCreateRenderPass(mDevice.logical_device(), &render_pass_info,
									nullptr, &mRenderPass));
	}

	void VulkanRenderer::init_sync_objects() {
		// create syncronization structures
		// one fence to control when the gpu has finished rendering the frame,
		// and 2 semaphores to syncronize rendering with swapchain
		// we want the fence to start signalled so we can wait on it on the
		// first frame
		for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
			VkFenceCreateInfo fenceCreateInfo =
				builder::fence_ci(VK_FENCE_CREATE_SIGNALED_BIT);
			VK_CHECK(vkCreateFence(mDevice.logical_device(), &fenceCreateInfo,
								   nullptr, &mFrames[i].render_fence));
			VkSemaphoreCreateInfo semaphoreCreateInfo = builder::semaphore_ci();
			VK_CHECK(vkCreateSemaphore(mDevice.logical_device(),
									   &semaphoreCreateInfo, nullptr,
									   &mFrames[i].present_semaphore));
			VK_CHECK(vkCreateSemaphore(mDevice.logical_device(),
									   &semaphoreCreateInfo, nullptr,
									   &mFrames[i].render_semaphore));
		}
		// no signaled bit because we don't have to wait
		VkFenceCreateInfo upload_fence_ci{builder::fence_ci()};
		VK_CHECK(vkCreateFence(mDevice.logical_device(), &upload_fence_ci,
							   nullptr, &mUploadContext.upload_fence));
	}

	void VulkanRenderer::init_descriptors() {
		mDescriptorAllocatorPool = {mDevice};
		mDescriptorLayoutCache = {mDevice};
		mMainDescriptorAllocator = mDescriptorAllocatorPool.get_allocator(0);
		const size_t scene_param_buffer_size =
			MAXIMUM_FRAMES_IN_FLIGHT * pad_uniform_buffer(sizeof(SceneData));
		mScene = {mDevice.allocator(), scene_param_buffer_size, {}};
		{
			for (int i = 0; i < MAXIMUM_FRAMES_IN_FLIGHT; ++i) {
				{
					mFrames[i].camera_buffer = {
						mDevice.allocator(), sizeof(CameraData),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VMA_MEMORY_USAGE_CPU_TO_GPU};
					VkDescriptorBufferInfo camera_buffer_info{
						mFrames[i].camera_buffer.handle, 0, sizeof(CameraData)};
					VkDescriptorBufferInfo scene_buffer_info =
						mScene.buffer_info(0);
					builder::DescriptorSetBuilder builder{
						mDevice, &mDescriptorLayoutCache,
						&mMainDescriptorAllocator};
					mFrames[i].global_descriptor = std::move(
						builder
							.add_buffer(0, &camera_buffer_info,
										VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
										VK_SHADER_STAGE_VERTEX_BIT)
							.add_buffer(
								1, &scene_buffer_info,
								VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
								VK_SHADER_STAGE_VERTEX_BIT |
									VK_SHADER_STAGE_FRAGMENT_BIT)
							.build()
							.value());
					mGlobalDescriptorSetLayout = builder.layout();
				}
				{
					mFrames[i].object_buffer = {
						mDevice.allocator(), sizeof(ObjectData) * MAX_OBJECTS,
						VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
						VMA_MEMORY_USAGE_CPU_TO_GPU};
					VkDescriptorBufferInfo object_buffer_info{
						mFrames[i].object_buffer.handle, 0,
						sizeof(ObjectData) * MAX_OBJECTS};
					builder::DescriptorSetBuilder builder{
						mDevice, &mDescriptorLayoutCache,
						&mMainDescriptorAllocator};
					mFrames[i].object_descriptor = std::move(
						builder
							.add_buffer(0, &object_buffer_info,
										VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
										VK_SHADER_STAGE_VERTEX_BIT)
							.build()
							.value());
					mObjectsDescriptorSetLayout = builder.layout();
				}
			}
		}
	}

	void VulkanRenderer::init_scene() {
		{
			mCamera = {glm::radians(70.f),
					   static_cast<f32>(mWindowExtent.width) /
						   mWindowExtent.height,
					   0.1f, 200.0f};
			mImageCache = {mDevice, this};
			mShaderCache = {mDevice};
			Material material{};
			builder::PipelineBuilder builder;
			material.layout =
				builder
					.add_shader_module(
						mShaderCache.get_shader(
							"assets/shaders/default_shader.vert.glsl.spv"),
						ShaderType::VERTEX)
					.add_shader_module(
						mShaderCache.get_shader(
							"assets/shaders/default_shader.frag.glsl.spv"),
						ShaderType::FRAGMENT)
					.set_vertex_input_description(Vertex::get_description())
					.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
										false)
					.set_polygon_mode(VK_POLYGON_MODE_FILL)
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
					.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
					.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
					/* .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH) */
					.add_viewport(
						{0, 0, static_cast<float>(mWindowExtent.width),
						 static_cast<float>(mWindowExtent.height), 0.f, 1.f})
					.add_scissor({{0, 0}, mWindowExtent})
					.build_layout(mDevice.logical_device());
			material.pipeline =
				builder.build_pipeline(mDevice.logical_device(), mRenderPass);
			Mesh monkeyMesh{};
			monkeyMesh.load_from_obj("assets/models/monkey.obj");
			upload_mesh(monkeyMesh);
			add_material_to_mesh(material, monkeyMesh);
		}
		{
			Material material{};
			{
				builder::DescriptorSetBuilder builder{
					mDevice, &mDescriptorLayoutCache,
					&mMainDescriptorAllocator};
				Image* image = mImageCache.get_image(
					"assets/textures/lost_empire-RGBA.png");
				VkDescriptorImageInfo image_buf_info{
					image->sampler(), image->view,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
				material.texture_set = std::move(
					builder
						.add_image(0, &image_buf_info,
								   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
								   VK_SHADER_STAGE_FRAGMENT_BIT)
						.build()
						.value());
				mTextureSamplerDescriptorSetLayout = builder.layout();
			}
			builder::PipelineBuilder builder;
			material.layout =
				builder
					.add_shader_module(
						mShaderCache.get_shader(
							"assets/shaders/textured_mesh.vert.glsl.spv"),
						ShaderType::VERTEX)
					.add_shader_module(
						mShaderCache.get_shader(
							"assets/shaders/textured_mesh.frag.glsl.spv"),
						ShaderType::FRAGMENT)
					.set_vertex_input_description(Vertex::get_description())
					.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
										false)
					.set_polygon_mode(VK_POLYGON_MODE_FILL)
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
					.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
					.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
					// .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH)
					.add_viewport(
						{0, 0, static_cast<float>(mWindowExtent.width),
						 static_cast<float>(mWindowExtent.height), 0.f, 1.f})
					.add_scissor({{0, 0}, mWindowExtent})
					.build_layout(mDevice.logical_device());
			material.pipeline =
				builder.build_pipeline(mDevice.logical_device(), mRenderPass);
			Mesh lost_empire{};
			lost_empire.load_from_obj("assets/models/lost_empire.obj");
			upload_mesh(lost_empire);
			add_material_to_mesh(material, lost_empire);
		}
		{
			Material material{};
			{
				Image* image = mImageCache.get_image(
					"assets/textures/cubemap_yokohama_rgba.ktx");
				builder::DescriptorSetBuilder builder{
					mDevice, &mDescriptorLayoutCache,
					&mMainDescriptorAllocator};
				VkDescriptorImageInfo image_buf_info{
					image->sampler(), image->view,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
				material.texture_set = std::move(
					builder
						.add_image(0, &image_buf_info,
								   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
								   VK_SHADER_STAGE_FRAGMENT_BIT)
						.build()
						.value());
			}
			builder::PipelineBuilder builder;
			material.layout =
				builder
					.add_shader_module(
						mShaderCache.get_shader(
							"assets/shaders/skybox.vert.glsl.spv"),
						ShaderType::VERTEX)
					.add_shader_module(
						mShaderCache.get_shader(
							"assets/shaders/skybox.frag.glsl.spv"),
						ShaderType::FRAGMENT)
					.set_vertex_input_description(Vertex::get_description())
					.set_input_assembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
										false)
					.set_polygon_mode(VK_POLYGON_MODE_FILL)
					.set_cull_mode(VK_CULL_MODE_FRONT_BIT,
								   VK_FRONT_FACE_CLOCKWISE)
					.set_multisampling_enabled(false)
					.add_default_color_blend_attachment()
					.set_color_blending_enabled(false)
					.add_push_constant(sizeof(MeshPushConstant),
									   VK_SHADER_STAGE_VERTEX_BIT)
					.add_descriptor_set_layout(mGlobalDescriptorSetLayout)
					.add_descriptor_set_layout(mObjectsDescriptorSetLayout)
					.add_descriptor_set_layout(
						mTextureSamplerDescriptorSetLayout)
					.set_depth_testing(true, false, VK_COMPARE_OP_LESS_OR_EQUAL)
					.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
					.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR)
					// .add_dynamic_state(VK_DYNAMIC_STATE_LINE_WIDTH)
					.add_viewport(
						{0, 0, static_cast<float>(mWindowExtent.width),
						 static_cast<float>(mWindowExtent.height), 0.f, 1.f})
					.add_scissor({{0, 0}, mWindowExtent})
					.build_layout(mDevice.logical_device());
			material.pipeline =
				builder.build_pipeline(mDevice.logical_device(), mRenderPass);
			Mesh skybox{};
			skybox.load_primitive(PrimitiveType::Cube);
			upload_mesh(skybox);
			add_material_to_mesh(material, skybox);
		}
		mScene.scene_data.ambient_color = {0.7f, 0.4f, 0.1f, 0.f};
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
		size_t minUboAlignment = mDevice.physical_device_properties()
									 .limits.minUniformBufferOffsetAlignment;
		size_t alignedSize = original_size;
		if (minUboAlignment > 0) {
			alignedSize =
				(alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		return alignedSize;
	}

	void VulkanRenderer::upload_mesh(Mesh& mesh) {
		const size_t buf_size = mesh.vertices.size() * sizeof(Vertex);
		Buffer staging{mDevice.allocator(), buf_size,
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					   VMA_MEMORY_USAGE_CPU_ONLY};
		void* data;
		vmaMapMemory(mDevice.allocator(), staging.memory, &data);
		memcpy(data, mesh.vertices.data(), buf_size);
		vmaUnmapMemory(mDevice.allocator(), staging.memory);
		mesh.buffer = {mDevice.allocator(), buf_size,
					   VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
						   VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					   VMA_MEMORY_USAGE_GPU_ONLY};
		immediate_submit([=](VkCommandBuffer cmd) {
			VkBufferCopy copy{0, 0, buf_size};
			vkCmdCopyBuffer(cmd, staging.handle, mesh.buffer.handle, 1, &copy);
		});
		staging.destroy();
	}

	VertexBuffer VulkanRenderer::merge_vertices(ArrayList<Mesh>& meshes) {
		auto joined_view = meshes |
			std::views::transform([](Mesh& mesh) -> ArrayList<Vertex>& {
							   return mesh.vertices;
						   }) |
			std::views::join;
		ArrayList<Vertex> merged_vertices;
		std::ranges::copy(joined_view, std::back_inserter(merged_vertices));
		const size_t buf_size =
			std::ranges::size(merged_vertices) * sizeof(Vertex);
		VertexBuffer vertex_buffer{static_cast<u32>(buf_size)};
		Buffer staging{mDevice.allocator(), buf_size,
					   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					   VMA_MEMORY_USAGE_CPU_ONLY};
		void* data;
		vmaMapMemory(mDevice.allocator(), staging.memory, &data);
		memcpy(data, merged_vertices.data(), buf_size);
		vmaUnmapMemory(mDevice.allocator(), staging.memory);
		vertex_buffer.buffer = {mDevice.allocator(), buf_size,
								VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
									VK_BUFFER_USAGE_TRANSFER_DST_BIT,
								VMA_MEMORY_USAGE_GPU_ONLY};
		immediate_submit([=](VkCommandBuffer cmd) {
			VkBufferCopy copy{0, 0, buf_size};
			vkCmdCopyBuffer(cmd, staging.handle, vertex_buffer.buffer.handle, 1,
							&copy);
		});
		staging.destroy();
		return vertex_buffer;
	}

	void VulkanRenderer::immediate_submit(
		std::function<void(VkCommandBuffer cmd)>&& fn) {
		VkCommandBuffer cmd = mUploadContext.command_buffer;
		VkCommandBufferBeginInfo begin_info{builder::command_buffer_begin_info(
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)};
		VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));
		fn(cmd);
		VK_CHECK(vkEndCommandBuffer(cmd));
		VkSubmitInfo submit{builder::submit_info(&cmd)};
		VK_CHECK(vkQueueSubmit(mDevice.graphics_queue(), 1, &submit,
							   mUploadContext.upload_fence));
		vkWaitForFences(mDevice.logical_device(), 1,
						&mUploadContext.upload_fence, true, 1000000000);
		vkResetFences(mDevice.logical_device(), 1,
					  &mUploadContext.upload_fence);
		vkResetCommandPool(mDevice.logical_device(),
						   mUploadContext.command_pool, 0);
	}
} // namespace render::vulkan
