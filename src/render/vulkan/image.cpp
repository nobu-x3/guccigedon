#include "render/vulkan/image.h"
#include <glm/ext/scalar_constants.hpp>
#include <memory>
#include <string_view>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "assets/textures/texture_importer.h"
#include "render/vulkan/builders.h"
#include "render/vulkan/renderer.h"
#include "render/vulkan/types.h"

namespace render::vulkan {

	Image::Image(VmaAllocator alloc, VkDevice device,
				 const VkImageCreateInfo& image_ci,
				 const VmaAllocationCreateInfo& alloc_info,
				 VkImageAspectFlags aspect_flags) :
		mAllocator(alloc),
		mDevice(device), mLifetime(ObjectLifetime::OWNED) {
		VK_CHECK(vmaCreateImage(alloc, &image_ci, &alloc_info, &handle, &memory,
								nullptr));
		VkImageViewCreateInfo view_ci =
			builder::imageview_ci(image_ci.format, handle, aspect_flags);
		VK_CHECK(vkCreateImageView(mDevice, &view_ci, nullptr, &view));
	}

	Image::Image(std::string_view path, VmaAllocator alloc, VkDevice device,
				 VulkanRenderer& renderer, bool create_sampler) :
		mAllocator(alloc),
		mDevice(device), mLifetime(ObjectLifetime::OWNED) {
		// NOTE: okay this is a hack until I implement asset manager
		std::unique_ptr<asset::Texture> texture =
			std::unique_ptr<asset::Texture>(
				std::move(asset::TextureImporter::import({path})));
		// rgba to match vk
		VkDeviceSize img_size = texture->size();
		Buffer staging_buffer{alloc, img_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							  VMA_MEMORY_USAGE_CPU_ONLY};
		void* data;
		vmaMapMemory(alloc, staging_buffer.memory, &data);
		memcpy(data, texture->pixels(), static_cast<size_t>(img_size));
		vmaUnmapMemory(alloc, staging_buffer.memory);
		VkExtent3D img_extent{static_cast<u32>(texture->width()),
							  static_cast<u32>(texture->height()), 1};
		VkImageCreateInfo image_ci = builder::image_ci(
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			img_extent, texture->mip_levels(), texture->layer_count());
		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		VK_CHECK(vmaCreateImage(alloc, &image_ci, &alloc_info, &handle, &memory,
								nullptr));
		renderer.immediate_submit([&](VkCommandBuffer buf) {
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = texture->mip_levels();
			range.baseArrayLayer = 0;
			range.layerCount = texture->layer_count();
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
			ArrayList<VkBufferImageCopy> buffer_copy_regions;
			for (int face = 0; face < texture->layer_count(); ++face) {
				for (int level = 0; level < texture->mip_levels(); ++level) {
					VkBufferImageCopy copy_region{};
					copy_region.imageSubresource.aspectMask =
						VK_IMAGE_ASPECT_COLOR_BIT;
					copy_region.imageSubresource.mipLevel = level;
					copy_region.imageSubresource.baseArrayLayer = face;
					copy_region.imageSubresource.layerCount = 1;
					copy_region.imageExtent.width = texture->width() >> level;
					copy_region.imageExtent.height = texture->height() >> level;
					copy_region.imageExtent.depth = 1;
					copy_region.bufferOffset = texture->offset(level, 0, face);
					buffer_copy_regions.push_back(copy_region);
				}
			}
			vkCmdCopyBufferToImage(buf, staging_buffer.handle, handle,
								   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								   static_cast<u32>(buffer_copy_regions.size()),
								   buffer_copy_regions.data());
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
			VK_FORMAT_R8G8B8A8_SRGB, handle, VK_IMAGE_ASPECT_COLOR_BIT,
			texture->mip_levels(), texture->layer_count());
		vkCreateImageView(device, &view_ci, nullptr, &view);
		if (create_sampler) {
			VkSamplerCreateInfo sampler_info = builder::sampler_create_info(
				VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
				texture->mip_levels());
			vkCreateSampler(device, &sampler_info, nullptr, &mSampler);
		}
	}

	Image::Image(const Image& other) {
		handle = other.handle;
		view = other.view;
		memory = other.memory;
		mAllocator = other.mAllocator;
		mDevice = other.mDevice;
		mSampler = other.mSampler;
		mLifetime = ObjectLifetime::TEMP;
	}

	Image& Image::operator=(const Image& other) {
		handle = other.handle;
		view = other.view;
		memory = other.memory;
		mAllocator = other.mAllocator;
		mDevice = other.mDevice;
		mSampler = other.mSampler;
		mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Image::Image(Image&& img) noexcept {
		handle = img.handle;
		view = img.view;
		memory = img.memory;
		mAllocator = img.mAllocator;
		mDevice = img.mDevice;
		mSampler = img.mSampler;
		mLifetime = ObjectLifetime::OWNED;
		img.mLifetime = ObjectLifetime::TEMP;
	}

	Image& Image::operator=(Image&& img) noexcept {
		handle = img.handle;
		view = img.view;
		memory = img.memory;
		mAllocator = img.mAllocator;
		mDevice = img.mDevice;
		mSampler = img.mSampler;
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
			if (mSampler) {
				vkDestroySampler(mDevice, mSampler, nullptr);
			}
		}
		if (mAllocator && handle && memory) {
			vmaDestroyImage(mAllocator, handle, memory);
		}
	}
	bool ImageCache::add_image(const std::string& path, VmaAllocator alloc,
							   VkDevice device,
							   const VkImageCreateInfo& image_ci,
							   const VmaAllocationCreateInfo& alloc_info,
							   VkImageAspectFlags aspectFlags) {
		if (mCache.contains(path)) {
			core::Logger::Warning("Texture on path {} is already in the cache.",
								  path);
			return false;
		}
		mCache[path] = {alloc, device, image_ci, alloc_info, aspectFlags};
		return true;
	}

	bool ImageCache::add_image(const std::string& path, VmaAllocator alloc,
							   VkDevice device, VulkanRenderer& renderer,
							   bool create_sampler) {
		if (mCache.contains(path)) {
			core::Logger::Warning("Texture on path {} is already in the cache.",
								  path);
			return false;
		}
		mCache[path] = {path, alloc, device, renderer, create_sampler};
		return true;
	}

	bool ImageCache::add_image(const std::string& path, Image& image) {
		if (mCache.contains(path)) {
			core::Logger::Warning("Texture on path {} is already in the cache.",
								  path);
			return false;
		}
		mCache[path] = image;
		return true;
	}

	Image* ImageCache::get_image(const std::string& path) {
		if (!mCache.contains(path)) {
			if (!mRenderer)
				return nullptr;
			mCache[path] = {path.c_str(), mAlloc, mDevice, *mRenderer, true};
		}
		return &mCache[path];
	}
} // namespace render::vulkan
