#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "core/core.h"
#include "render/vulkan/device.h"
#include "render/vulkan/instance.h"
#include "render/vulkan/surface.h"
#include "render/vulkan/swapchain.h"
#include "scene.h"
#include "vulkan_image.h"
#include "vulkan_mesh.h"
#include "vulkan_types.h"

struct SDL_Window;

namespace render::vulkan {

	constexpr u32 MAXIMUM_FRAMES_IN_FLIGHT = 2;
	constexpr u32 MAX_OBJECTS = 1000;

	class VulkanRenderer {

		// I'm obviously not gonna keep all this in this megaclass.
		// This is temporary, I'll refactor once I get it running.

	private:
		VkExtent2D mWindowExtent{800, 600};
		SDL_Window* mpWindow{nullptr};
		Instance mInstance{};
        Device mDevice{};
        Surface mSurface{};
        Swapchain mSwapchain{};
		VkRenderPass mRenderPass{};
		HashMap<Material, ArrayList<Mesh>> mMaterialMap{};
		FrameData mFrames[MAXIMUM_FRAMES_IN_FLIGHT];
		VkDescriptorSetLayout mGlobalDescriptorSetLayout{};
		VkDescriptorSetLayout mObjectsDescriptorSetLayout{};
		VkDescriptorSetLayout mTextureSamplerDescriptorSetLayout{};
		VkDescriptorPool mDescriptorPool{};
		Scene mScene{};
		UploadContext mUploadContext{};
		u32 mCurrFrame{0};

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

		void upload_mesh(Mesh& mesh);

		// function template instead of std::function because c++20...
		// @TODO: figure out lambdas as param and passing this fn as pointer
		void
		immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

		inline FrameData& get_current_frame() {
			return mFrames[mCurrFrame % MAXIMUM_FRAMES_IN_FLIGHT];
		}

		void draw();
		void run();
	};
} // namespace render::vulkan
