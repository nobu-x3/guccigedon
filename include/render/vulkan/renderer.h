#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>
#include "assets/scene/gltf_importer.h"
#include "gameplay/camera.h"
#include "gameplay/transform.h"
#include "render/vulkan/descriptor_allocator.h"
#include "render/vulkan/descriptor_set_builder.h"
#include "render/vulkan/device.h"
#include "render/vulkan/image.h"
#include "render/vulkan/instance.h"
#include "render/vulkan/mesh.h"
#include "render/vulkan/scene.h"
#include "render/vulkan/shader.h"
#include "render/vulkan/surface.h"
#include "render/vulkan/swapchain.h"
#include "render/vulkan/types.h"

struct SDL_Window;

namespace render::vulkan {

	constexpr u32 MAXIMUM_FRAMES_IN_FLIGHT = 2;
	constexpr u32 MAX_OBJECTS = 1000;

	struct VertexBuffer {
		u32 size{0};
		Buffer buffer{};
	};

	class VulkanRenderer {

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
		void begin_renderpass(FrameData& frame_data, u32& image_index);
		void end_renderpass(VkCommandBuffer buf);
		void resize();

	public:
		VulkanRenderer();
		VulkanRenderer(std::filesystem::path path);
		VulkanRenderer(const asset::GLTFImporter& scene_asset);
		~VulkanRenderer();
		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;
		VulkanRenderer(VulkanRenderer&&) noexcept;
		VulkanRenderer& operator=(VulkanRenderer&&) noexcept;

		void add_material_to_mesh(const Material& material, const Mesh& mesh);

		void upload_mesh(Mesh& mesh);
		VertexBuffer merge_vertices(ArrayList<Mesh>&);

		// function template instead of std::function because c++20...
		// @TODO: figure out lambdas as param and passing this fn as pointer
		void
		immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

		inline FrameData& get_current_frame() {
			return mFrames[mCurrFrame % MAXIMUM_FRAMES_IN_FLIGHT];
		}

		void handle_input_event(core::PollResult& poll_result);
		void draw(const ArrayList<gameplay::Transform>& transforms);

		inline SDL_Window* window() const { return mpWindow; }

		inline ImageCache& image_cache() { return mImageCache; }

		inline ShaderCache& shader_cache() { return mShaderCache; }

		inline const Device& device() const { return mDevice; }

		inline DescriptorLayoutCache& descriptor_layout_cache() {
			return mDescriptorLayoutCache;
		}

		inline DescriptorAllocator& main_descriptor_allocator() {
			return mMainDescriptorAllocator;
		}

		inline VkDescriptorSetLayout global_descriptor_layout() const {
			return mGlobalDescriptorSetLayout;
		}

		inline VkDescriptorSetLayout objects_descriptor_layout() const {
			return mObjectsDescriptorSetLayout;
		}

		inline VkExtent2D window_extent() const { return mWindowExtent; }

		inline VkRenderPass render_pass() const { return mRenderPass; }

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
		HashMap<Material, VertexBuffer> mMaterialBufferMap{};
		FrameData mFrames[MAXIMUM_FRAMES_IN_FLIGHT];
		DescriptorAllocatorPool mDescriptorAllocatorPool{};
		DescriptorAllocator mMainDescriptorAllocator{};
		DescriptorLayoutCache mDescriptorLayoutCache{};
		VkDescriptorSetLayout mGlobalDescriptorSetLayout{};
		VkDescriptorSetLayout mObjectsDescriptorSetLayout{};
		VkDescriptorSetLayout mTextureSamplerDescriptorSetLayout{};
		Scene mScene{};
		GLTFModel mGltfScene;
		UploadContext mUploadContext{};
		u32 mCurrFrame{0};
		bool mShouldResize{false};
		ShaderCache mShaderCache{};
		gameplay::Camera mCamera{};
		ImageCache mImageCache{};
	};
} // namespace render::vulkan
