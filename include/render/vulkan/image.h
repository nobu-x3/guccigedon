#pragma once

#include <functional>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/types.h"

namespace render::vulkan {
	class VulkanRenderer;

	class Image {
	public:
		VkImage handle{};
		VkImageView view{};
		VmaAllocation memory{};

	private:
		VmaAllocator mAllocator{};
		VkDevice mDevice{};
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};

	public:
		Image() = default;

		// for depth
		Image(VmaAllocator alloc, VkDevice device,
			  const VkImageCreateInfo& image_ci,
			  const VmaAllocationCreateInfo& alloc_info,
			  VkImageAspectFlags aspectFlags);

		// the function pointer is a mess @TODO
		// instead of passing a reference to renderer, pass pfn
		Image(const char* path, VmaAllocator alloc, VkDevice device,
			  VulkanRenderer& renderer);

		Image& operator=(Image& other);

		Image(Image&& img);

		Image& operator=(Image&& img);

		~Image();
	};
} // namespace render::vulkan
