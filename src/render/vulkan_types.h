#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vk_mem_alloc.h>
#include "../core/logger.h"

#define VK_CHECK(x)                                                            \
	do {                                                                       \
		VkResult err = x;                                                      \
		if (err) {                                                             \
			core::Logger::Error("Detected Vulkan error: %s", err);             \
			exit(-1);                                                          \
		}                                                                      \
	} while (0)
