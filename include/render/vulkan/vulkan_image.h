#pragma once
#include <functional>
#include "vulkan/vulkan_core.h"
#include "vulkan_types.h"

namespace render {
	class VulkanRenderer;
	
	class Image {
	public:
		VkImage handle{};
		VkImageView view{};
		VmaAllocation memory{};

	private:
		VmaAllocator allocator{};
		VkDevice device{};

	public:
		Image() = default;

		Image(VmaAllocator alloc, VkDevice device,
			  const VkImageCreateInfo& image_ci,
			  const VmaAllocationCreateInfo& alloc_info);

		// the function pointer is a mess @TODO
		// instead of passing a reference to renderer, pass pfn
		Image(const char* path, VmaAllocator alloc, VkDevice device,
			  VulkanRenderer& renderer);

		void destroy();

		// @NOTE: imo a more elegant solution to throwing exception from
		// constructor
		static Result<Image>
		create_image(VmaAllocator alloc, VkDevice device,
					 const VkImageCreateInfo& image_ci,
					 const VmaAllocationCreateInfo& alloc_info) {
			Image image{};
			image.allocator = alloc;
			image.device = device;
			Result<Image> res{image};
			res = vmaCreateImage(alloc, &image_ci, &alloc_info,
								 &res.value().handle, &res.value().memory,
								 nullptr);
			if (!res.has_value()) {
				res->destroy(); // I think this works?
				core::Logger::Error("Failed to create image. %d",
									res.vk_result());
				return res;
			}
			return res;
		}

		bool create_view(const VkImageViewCreateInfo& view_ci) {
			VkResult res = vkCreateImageView(device, &view_ci, nullptr, &view);
			if (res != VK_SUCCESS) {
				core::Logger::Error("Failed to create image view. %d", res);
				return false;
			}
			return true;
		}
	};
} // namespace render
