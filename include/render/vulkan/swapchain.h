#pragma once

#include "render/vulkan/device.h"
#include "render/vulkan/surface.h"
#include "render/vulkan/vulkan_image.h"
#include "render/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"
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

		void init_framebuffers(VkRenderPass renderpass, VkExtent2D* extent = nullptr);

		inline VkSwapchainKHR& handle() { return mSwapchain; }

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
		VkDevice mDevice{};
		VkExtent2D mWindowExtent{};
	};
} // namespace render::vulkan
