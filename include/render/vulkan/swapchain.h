#pragma once

#include <vulkan/vulkan_core.h>
#include "render/vulkan/device.h"
#include "render/vulkan/image.h"
#include "render/vulkan/surface.h"
#include "render/vulkan/types.h"

namespace render::vulkan {

	class Swapchain {
	public:
		Swapchain() = default;
		// This allocates framebuffers
		Swapchain(VmaAllocator allocator, Device& device, Surface& surface,
				  VkExtent2D window_extent, VkRenderPass renderpass);
		// This does not allocate framebuffers
		Swapchain(VmaAllocator allocator, Device& device, Surface& surface,
				  VkExtent2D window_extent);
		Swapchain(Swapchain& swapchain);
		Swapchain(Swapchain&& swapchain) noexcept;
		Swapchain& operator=(Swapchain& swapchain);
		Swapchain& operator=(Swapchain&& swapchain) noexcept;
		~Swapchain();

		void rebuild(u32 width, u32 height, VkRenderPass renderpass);

		void init_framebuffers(VkRenderPass renderpass,
							   VkExtent2D* extent = nullptr);

		inline VkSwapchainKHR& handle() { return mSwapchain; }

		inline ObjectLifetime lifetime() { return mLifetime; }

		inline const ArrayList<VkImage>& images() const {
			return mSwapchainImages;
		}

		inline const ArrayList<VkImageView>& views() const {
			return mSwapchainImageViews;
		}

		inline const ArrayList<VkFramebuffer>& framebuffers() const {
			return mFramebuffers;
		}

		inline VkFormat image_format() const { return mSwapchainImageFormat; }

		inline VkFormat depth_format() const { return mDepthFormat; }

	private:
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};
		VkFormat mSwapchainImageFormat{};
		VkSwapchainKHR mSwapchain{};
		ArrayList<VkImage> mSwapchainImages{};
		ArrayList<VkImageView> mSwapchainImageViews{};
		ArrayList<VkFramebuffer> mFramebuffers{};
		VkFormat mDepthFormat{VK_FORMAT_D32_SFLOAT};
		Image mDepthAttachment{};
		VkDevice mDevice{}; // non-owner
		VkPhysicalDevice mPhysicalDevice{}; // non-owner
		VkExtent2D mWindowExtent{}; // non-owner
		VkSurfaceKHR mSurface{}; // non-owner
		VmaAllocator mAllocator{};
	};
} // namespace render::vulkan
