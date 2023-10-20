#pragma once
#include <algorithm>
#include <cstring>
#include <glm/ext/vector_float3.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
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
    VkBuffer handle;
    VmaAllocation memory;
};


struct Material {
    VkPipeline pipeline;
    VkPipelineLayout layout;
};
