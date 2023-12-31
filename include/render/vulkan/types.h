#pragma once
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "core/core.h"

namespace render::vulkan {

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			core::Logger::Error("Detected Vulkan error: {} in {} at {}",       \
								(u32)err, __FILE__, __LINE__);                 \
			std::abort();                                                      \
		}                                                                      \
	} while (0)

	class Buffer {

	public:
		VkBuffer handle{};
		VmaAllocation memory{};

	public:
		Buffer() = default;
		Buffer(VmaAllocator alloc, size_t allocation_size,
			   VkBufferUsageFlags usage, VmaMemoryUsage mem_usage);
		Buffer(const Buffer& other);
		Buffer& operator=(const Buffer& other);
		Buffer(Buffer&& other) noexcept;
		Buffer& operator=(Buffer&& other) noexcept;

		// The reason we want to explicitly destroy the object is because
		// most of the time we don't want to actually clear the memory and
		// destroy the handle when object's lifetime is over. Consider copy
		// ctor/assignment: causes double free.
		void destroy();

	private:
		VmaAllocator mAlloc;
	};

	// TODO: make this a RAII class, remove stuff from scene.cpp dtor
	struct Material {
		VkDescriptorSet texture_set{VK_NULL_HANDLE};
		VkPipeline pipeline{};
		VkPipelineLayout layout{};
		glm::vec4 base_color_factor = glm::vec4(1.f);
		u32 base_color_texture_index;

		bool operator==(const Material& other) const {
			return pipeline == other.pipeline;
		}
	};

	struct MeshPushConstant {
        u32 id;
	};

	struct FrameData {
		VkCommandPool command_pool{};
		VkCommandBuffer command_buffer{};
		VkSemaphore present_semaphore{}, render_semaphore{};
		VkFence render_fence{};
		Buffer camera_buffer{};
		VkDescriptorSet global_descriptor{};
		Buffer object_buffer{};
		VkDescriptorSet object_descriptor{};
	};

	struct UploadContext {
		VkFence upload_fence;
		VkCommandPool command_pool;
		VkCommandBuffer command_buffer;
	};

	struct CameraData {
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 view_proj;
	};

	struct SceneData {
		glm::vec4 ambient_color;
	};

	struct ObjectData {
		glm::mat4 model_matrix;
	};

	struct VertexInputDescription {
		ArrayList<VkVertexInputBindingDescription> bindings;
		ArrayList<VkVertexInputAttributeDescription> attributes;
		VkPipelineVertexInputStateCreateFlags flags{0};
	};

	struct Vertex {
		glm::vec3 position{0, 0, 0};
		glm::vec3 color{0, 0, 0};
		glm::vec3 normal{0, 0, 0};
		glm::vec2 uv{0, 0};

		static VertexInputDescription get_description();
	};

	struct SkyboxVertex {
		glm::vec3 position;

		static VertexInputDescription get_description();
	};
} // namespace render::vulkan

template <>
struct std::hash<render::vulkan::Material> {
	std::size_t operator()(const render::vulkan::Material& k) const {
		return hash<void*>()(k.pipeline);
	}
};
