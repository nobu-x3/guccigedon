#include <glm/ext/scalar_constants.hpp>
#include <vk_mem_alloc.h>
#include "render/vulkan/types.h"
#include <vulkan/vulkan_core.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "render/vulkan/renderer.h"
#include "render/vulkan/builders.h"
#include "render/vulkan/image.h"

namespace render::vulkan {

	Image::Image(VmaAllocator alloc, VkDevice device,
				 const VkImageCreateInfo& image_ci,
				 const VmaAllocationCreateInfo& alloc_info,
				 VkImageAspectFlags aspect_flags) :
		mAllocator(alloc),
		mDevice(device), mLifetime(ObjectLifetime::OWNED) {
		VK_CHECK(vmaCreateImage(alloc, &image_ci, &alloc_info, &handle, &memory,
								nullptr));
        VkImageViewCreateInfo view_ci = builder::imageview_ci(image_ci.format, handle, aspect_flags);
		VK_CHECK(vkCreateImageView(mDevice, &view_ci, nullptr, &view));
	}

	Image::Image(const char* path, VmaAllocator alloc, VkDevice device,
				 VulkanRenderer& renderer) :
		mAllocator(alloc),
		mDevice(device), mLifetime(ObjectLifetime::OWNED) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels,
									STBI_rgb_alpha);
		if (!pixels) {
			// @TODO: load default texture instead of exiting
			core::Logger::Fatal("Failed to load data from image at path %s",
								path);
			exit(-1);
		}
		void* pPixels = pixels;
		// rgba to match vk
		VkDeviceSize img_size = texWidth * texHeight * 4;
		Buffer staging_buffer{alloc, img_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							  VMA_MEMORY_USAGE_CPU_ONLY};
		void* data;
		vmaMapMemory(alloc, staging_buffer.memory, &data);
		memcpy(data, pPixels, static_cast<size_t>(img_size));
		vmaUnmapMemory(alloc, staging_buffer.memory);
		stbi_image_free(pixels);
		VkExtent3D img_extent{static_cast<u32>(texWidth),
							  static_cast<u32>(texHeight), 1};
		VkImageCreateInfo image_ci = builder::image_ci(
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			img_extent);
		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		VK_CHECK(vmaCreateImage(alloc, &image_ci, &alloc_info, &handle, &memory,
								nullptr));
		renderer.immediate_submit([&](VkCommandBuffer buf) {
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;
			VkImageMemoryBarrier img_barrier_transfer{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				0,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL};
			img_barrier_transfer.image = handle;
			img_barrier_transfer.subresourceRange = range;
			vkCmdPipelineBarrier(buf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
								 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
								 0, nullptr, 1, &img_barrier_transfer);
			VkBufferImageCopy copy_region{
				0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {}, img_extent};
			vkCmdCopyBufferToImage(buf, staging_buffer.handle, handle,
								   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
								   &copy_region);
			VkImageMemoryBarrier img_barrier_shader_readable =
				img_barrier_transfer;
			img_barrier_shader_readable.oldLayout =
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			img_barrier_shader_readable.newLayout =
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			img_barrier_shader_readable.srcAccessMask =
				VK_ACCESS_TRANSFER_WRITE_BIT;
			img_barrier_shader_readable.dstAccessMask =
				VK_ACCESS_SHADER_READ_BIT;
			// barrier the image into the shader readable layout
			vkCmdPipelineBarrier(buf, VK_PIPELINE_STAGE_TRANSFER_BIT,
								 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
								 nullptr, 0, nullptr, 1,
								 &img_barrier_shader_readable);
		});
		staging_buffer.destroy();
		VkImageViewCreateInfo view_ci = builder::imageview_ci(
			VK_FORMAT_R8G8B8A8_SRGB, handle, VK_IMAGE_ASPECT_COLOR_BIT);
		vkCreateImageView(device, &view_ci, nullptr, &view);
	}

	Image& Image::operator=(Image& other) {
		handle = other.handle;
		view = other.view;
		memory = other.memory;
		mAllocator = other.mAllocator;
		mDevice = other.mDevice;
		mLifetime = ObjectLifetime::OWNED;
		other.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Image::Image(Image&& img) {
		handle = img.handle;
		view = img.view;
		memory = img.memory;
		mAllocator = img.mAllocator;
		mDevice = img.mDevice;
		mLifetime = ObjectLifetime::OWNED;
		img.mLifetime = ObjectLifetime::TEMP;
	}

	Image& Image::operator=(Image&& img) {
		handle = img.handle;
		view = img.view;
		memory = img.memory;
		mAllocator = img.mAllocator;
		mDevice = img.mDevice;
		mLifetime = ObjectLifetime::OWNED;
		img.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Image::~Image() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (mDevice) {
			if (view) {
				vkDestroyImageView(mDevice, view, nullptr);
			}
		}
		if (mAllocator && handle && memory) {
			vmaDestroyImage(mAllocator, handle, memory);
		}
	}
} // namespace render