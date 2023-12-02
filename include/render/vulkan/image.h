#pragma once

#include <filesystem>
#include <functional>
#include <vulkan/vulkan_core.h>
#include "render/vulkan/device.h"
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
		VkSampler mSampler{nullptr};

	public:
		Image() = default;

		// for depth
		Image(VmaAllocator alloc, VkDevice device,
			  const VkImageCreateInfo& image_ci,
			  const VmaAllocationCreateInfo& alloc_info,
			  VkImageAspectFlags aspectFlags);

		// the function pointer is a mess @TODO
		// instead of passing a reference to renderer, pass pfn
		Image(std::string_view path, VmaAllocator alloc, VkDevice device,
			  VulkanRenderer& renderer, bool create_sampler = false);

		Image(const Image& other);

		Image& operator=(const Image& other);

		Image(Image&& img) noexcept;

		Image& operator=(Image&& img) noexcept;

		~Image();

		inline const VkSampler& sampler() const { return mSampler; }
	};

	class ImageCache {
	public:
		ImageCache() = default;
		ImageCache(Device& device, VulkanRenderer* renderer) :
			mDevice(device.logical_device()), mAlloc(device.allocator()),
			mRenderer(renderer) {}
		~ImageCache() = default;
		bool add_image(const std::string& path, VmaAllocator alloc,
					   VkDevice device, const VkImageCreateInfo& image_ci,
					   const VmaAllocationCreateInfo& alloc_info,
					   VkImageAspectFlags aspectFlags);
		bool add_image(const std::string& path, VmaAllocator alloc,
					   VkDevice device, VulkanRenderer& renderer,
					   bool create_sampler = false);
		bool add_image(const std::string& path, Image& image);

		// by default this will create a sampler and color attachment
		Image* get_image(const std::string& path);

	private:
		HashMap<std::string, Image> mCache{};
		VkDevice mDevice{};
		VmaAllocator mAlloc{};
		VulkanRenderer* mRenderer; // NOTE: temp
	};
} // namespace render::vulkan

template <>
struct std::hash<render::vulkan::Image> {
	std::size_t operator()(const render::vulkan::Image& k) const {
		return hash<void*>()(k.handle);
	}
};
