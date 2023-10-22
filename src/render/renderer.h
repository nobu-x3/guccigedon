#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "../core/core.h"
#include "scene.h"
#include "vulkan_image.h"
#include "vulkan_mesh.h"
#include "vulkan_types.h"

struct SDL_Window;

namespace render {

	constexpr u32 MAXIMUM_FRAMES_IN_FLIGHT = 2;

	struct RenderObject {
		Mesh mesh;
		Material Material;
	};

	class VulkanRenderer {

		// I'm obviously not gonna keep all this in this megaclass.
		// This is temporary, I'll refactor once I get it running.

	private:
		u32 mCurrFrame{0};
		VkExtent2D mWindowExtent{800, 600};
		SDL_Window* mpWindow{nullptr};
		VkInstance mInstance{};
		VkDebugUtilsMessengerEXT fpDebugMsger{};
		VkPhysicalDevice mPhysicalDevice{};
        VkPhysicalDeviceProperties mPhysicalDeviceProperties{};
		VkDevice mDevice{};
		VkSurfaceKHR mSurface{};
		VkQueue mGraphicsQueue{};
		u32 mGraphicsQueueFamily{};
		VkFormat mSwapchainImageFormat{};
		VkSwapchainKHR mSwapchain{};
		ArrayList<VkImage> mSwapchainImages{};
		ArrayList<VkImageView> mSwapchainImageViews{};
		ArrayList<VkFramebuffer> mFramebuffers{};
		VkRenderPass mRenderPass{};
		VmaAllocator mAllocator{};
		VkFormat mDepthFormat{VK_FORMAT_D32_SFLOAT};
		Image mDepthAttachment{};
		HashMap<Material, ArrayList<Mesh>> mMaterialMap{};
		FrameData mFrames[MAXIMUM_FRAMES_IN_FLIGHT];
        VkDescriptorSetLayout mGlobalDescriptorSetLayout{};
        VkDescriptorPool mDescriptorPool{};
        Scene mScene{};

	private:
		void init_instance();
		void init_swapchain();
		void init_commands();
		void init_framebuffers();
		void init_default_renderpass();
		void init_sync_objects();
        void init_descriptors();
		void init_scene();
        size_t pad_uniform_buffer(size_t original_size);

	public:
		VulkanRenderer();
		~VulkanRenderer();
		void add_material_to_mesh(const Material& material, const Mesh& mesh);

		inline FrameData& get_current_frame() {
			return mFrames[mCurrFrame % MAXIMUM_FRAMES_IN_FLIGHT];
		}

		void draw();
		void run();
	};
} // namespace render
