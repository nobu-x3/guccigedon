#pragma once
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../core/core.h"
namespace render {

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			core::Logger::Error("Detected Vulkan error: %s", err);             \
			exit(-1);                                                          \
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

		// The reason we want to explicitly destroy the object is because
		// most of the time we don't want to actually clear the memory and
		// destroy the handle when object's lifetime is over. Consider copy
		// ctor/assignment: causes double free.
		void destroy();

	private:
		VmaAllocator mAlloc;
	};

	struct Material {
		VkPipeline pipeline{};
		VkPipelineLayout layout{};

		bool operator==(const Material& other) const {
			return pipeline == other.pipeline;
		}
	};

	struct MeshPushConstant {
		glm::vec4 data{};
		glm::mat4 render_matrix{};
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
} // namespace render
  //
template <>
struct std::hash<render::Material> {
	std::size_t operator()(const render::Material& k) const {
		return hash<void*>()(k.pipeline);
	}
};
