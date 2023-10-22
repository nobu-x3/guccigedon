#pragma once
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <glm/glm.hpp>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../core/core.h"

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			core::Logger::Error("Detected Vulkan error: %s", err);             \
			exit(-1);                                                          \
		}                                                                      \
	} while (0)

struct Buffer {
	VkBuffer handle{};
	VmaAllocation memory{};
};

struct Material {
	VkPipeline pipeline{};
	VkPipelineLayout layout{};

	bool operator==(const Material& other) const {
		return pipeline == other.pipeline;
	}
};

template <>
struct std::hash<Material> {
	std::size_t operator()(const Material& k) const {
		return hash<void*>()(k.pipeline);
	}
};

struct MeshPushConstant {
	glm::vec4 data{};
	glm::mat4 render_matrix{};
};
