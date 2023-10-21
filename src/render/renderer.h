#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "../core/core.h"
#include "vulkan_mesh.h"
#include "vulkan_types.h"

struct SDL_Window;

namespace render {

	struct RenderObject {
		Mesh mesh;
		Material Material;
	};

	class VulkanRenderer {

	public:
		int mCurrFrame{0};
		VkExtent2D mWindowExtent{800, 600};
		SDL_Window* mpWindow{nullptr};
		VkInstance mInstance;
		VkDebugUtilsMessengerEXT fpDebugMsger;
		VkPhysicalDevice mPhysicalDevice;
		VkDevice mDevice;
		VkSurfaceKHR mSurface;
		VkQueue mGraphicsQueue;
		u32 mGraphicsQueueFamily;
		VkFormat mSwapchainImageFormat;
		VkSwapchainKHR mSwapchain;
		ArrayList<VkImage> mSwapchainImages{};
		ArrayList<VkImageView> mSwapchainImageViews{};
		ArrayList<VkFramebuffer> mFramebuffers{};
		VkCommandPool mCommandPool;
		VkCommandBuffer mMainCommandBuffer;
		VkSemaphore mPresentSemaphore, mRenderSemaphore;
		VkFence mRenderFence;
		VkRenderPass mRenderPass;
		VkPipelineLayout mGraphicsPipelineLayout;
		VkPipeline mGraphicsPipeline;
		VmaAllocator mAllocator;
		Mesh mMesh {};
        Mesh mMonkeyMesh {};

	private:
		void init_instance();
		void init_swapchain();
		void init_commands();
		void init_framebuffers();
		void init_default_renderpass();
		void init_sync_objects();
		void init_pipeline();
		void load_mesh();

	public:
		VulkanRenderer();
		~VulkanRenderer();
		void draw();
		void run();
	};
} // namespace render
