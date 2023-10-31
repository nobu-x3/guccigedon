#include "render/vulkan/swapchain.h"
#include <VkBootstrap.h>
#include "core/logger.h"
#include "render/vulkan/builders.h"
#include "render/vulkan/types.h"

namespace render::vulkan {

	Swapchain::Swapchain(VmaAllocator allocator, Device& device,
						 Surface& surface, VkExtent2D window_extent,
						 VkRenderPass renderpass) :
		mDevice(device.logical_device()),
		mWindowExtent(window_extent), mLifetime(ObjectLifetime::OWNED) {
		vkb::SwapchainBuilder swap_buider{device.physical_device(),
										  device.logical_device(),
										  surface.surface()};
		vkb::Swapchain vkb_swap =
			swap_buider.use_default_format_selection()
				.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
				.set_desired_extent(window_extent.width, window_extent.height)
				.build()
				.value();
		mSwapchain = vkb_swap.swapchain;
		mSwapchainImages = vkb_swap.get_images().value();
		mSwapchainImageViews = vkb_swap.get_image_views().value();
		mSwapchainImageFormat = vkb_swap.image_format;
		VkExtent3D depthImageExtent = {window_extent.width,
									   window_extent.height, 1};
		VmaAllocationCreateInfo imgAllocCi{};
		imgAllocCi.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		imgAllocCi.requiredFlags =
			VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mDepthAttachment = {
			allocator, device.logical_device(),
			vkbuild::image_ci(mDepthFormat,
							  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
							  depthImageExtent),
			imgAllocCi, VK_IMAGE_ASPECT_DEPTH_BIT};
		VkFramebufferCreateInfo fb_ci =
			vkbuild::framebuffer_ci(renderpass, mWindowExtent);
		const u32 image_count = mSwapchainImages.size();
		mFramebuffers = std::vector<VkFramebuffer>(image_count);
		for (int i = 0; i < image_count; ++i) {
			VkImageView attachments[2]{mSwapchainImageViews[i],
									   mDepthAttachment.view};
			fb_ci.pAttachments = &attachments[0];
			fb_ci.attachmentCount = 2;
			VK_CHECK(vkCreateFramebuffer(mDevice, &fb_ci, nullptr,
										 &mFramebuffers[i]));
		}
	}

	Swapchain::Swapchain(VmaAllocator allocator, Device& device,
						 Surface& surface, VkExtent2D window_extent) :
		mDevice(device.logical_device()),
		mWindowExtent(window_extent), mLifetime(ObjectLifetime::OWNED) {
		vkb::SwapchainBuilder swap_buider{device.physical_device(),
										  device.logical_device(),
										  surface.surface()};
		vkb::Swapchain vkb_swap =
			swap_buider.use_default_format_selection()
				.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
				.set_desired_extent(window_extent.width, window_extent.height)
				.build()
				.value();
		mSwapchain = vkb_swap.swapchain;
		mSwapchainImages = vkb_swap.get_images().value();
		mSwapchainImageViews = vkb_swap.get_image_views().value();
		mSwapchainImageFormat = vkb_swap.image_format;
		VkExtent3D depthImageExtent = {window_extent.width,
									   window_extent.height, 1};
		VmaAllocationCreateInfo imgAllocCi{};
		imgAllocCi.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		imgAllocCi.requiredFlags =
			VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mDepthAttachment = {
			allocator, device.logical_device(),
			vkbuild::image_ci(mDepthFormat,
							  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
							  depthImageExtent),
			imgAllocCi, VK_IMAGE_ASPECT_DEPTH_BIT};
	}

	Swapchain::Swapchain(Swapchain& swapchain) :
		mLifetime(ObjectLifetime::OWNED), mDevice(swapchain.mDevice),
		mWindowExtent(swapchain.mWindowExtent) {
		mSwapchain = swapchain.mSwapchain;
		mDepthFormat = swapchain.mDepthFormat;
		mDepthAttachment = swapchain.mDepthAttachment;
		mSwapchainImages = swapchain.mSwapchainImages;
		mSwapchainImageViews = swapchain.mSwapchainImageViews;
		mSwapchainImageFormat = swapchain.mSwapchainImageFormat;
		mFramebuffers = swapchain.mFramebuffers;
		swapchain.mLifetime = ObjectLifetime::TEMP;
	}

	Swapchain::Swapchain(Swapchain&& swapchain) noexcept :
		mLifetime(ObjectLifetime::OWNED), mDevice(swapchain.mDevice),
		mWindowExtent(swapchain.mWindowExtent) {
		mSwapchain = swapchain.mSwapchain;
		mDepthFormat = swapchain.mDepthFormat;
		mDepthAttachment = swapchain.mDepthAttachment;
		std::swap(mSwapchainImages, swapchain.mSwapchainImages);
		std::swap(mSwapchainImageViews, swapchain.mSwapchainImageViews);
		std::swap(mFramebuffers, swapchain.mFramebuffers);
		mSwapchainImageFormat = swapchain.mSwapchainImageFormat;
		swapchain.mLifetime = ObjectLifetime::TEMP;
	}

	Swapchain& Swapchain::operator=(Swapchain& swapchain) {
		mSwapchain = swapchain.mSwapchain;
		mWindowExtent = swapchain.mWindowExtent;
		mDepthFormat = swapchain.mDepthFormat;
		mDepthAttachment = swapchain.mDepthAttachment;
		mSwapchainImages = swapchain.mSwapchainImages;
		mSwapchainImageViews = swapchain.mSwapchainImageViews;
		mSwapchainImageFormat = swapchain.mSwapchainImageFormat;
		mFramebuffers = swapchain.mFramebuffers;
		mDevice = swapchain.mDevice;
        mLifetime = ObjectLifetime::OWNED;
		swapchain.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Swapchain& Swapchain::operator=(Swapchain&& swapchain) noexcept {
		mSwapchain = swapchain.mSwapchain;
		mDepthFormat = swapchain.mDepthFormat;
		mDepthAttachment = swapchain.mDepthAttachment;
		mWindowExtent = swapchain.mWindowExtent;
		std::swap(mSwapchainImages, swapchain.mSwapchainImages);
		std::swap(mSwapchainImageViews, swapchain.mSwapchainImageViews);
		std::swap(mFramebuffers, swapchain.mFramebuffers);
		mSwapchainImageFormat = swapchain.mSwapchainImageFormat;
		swapchain.mLifetime = ObjectLifetime::TEMP;
        mLifetime = ObjectLifetime::OWNED;
		mDevice = swapchain.mDevice;
		return *this;
	}

	Swapchain::~Swapchain() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (mSwapchain) {
			vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
		}
		for (int i = 0; i < mFramebuffers.size(); ++i) {
			if (mFramebuffers[i])
				vkDestroyFramebuffer(mDevice, mFramebuffers[i], nullptr);
			if (mSwapchainImageViews[i])
				vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);
		}
	}

	void Swapchain::init_framebuffers(VkRenderPass renderpass,
									  VkExtent2D* extent) {
		if (extent != nullptr) // dunno if we should do this, potentially bug
							   // prone because if extent != mWindowExtent we
							   // should also rebuild the swapchain
			mWindowExtent = *extent;
		VkFramebufferCreateInfo fb_ci =
			vkbuild::framebuffer_ci(renderpass, mWindowExtent);
		const u32 image_count = mSwapchainImages.size();
		mFramebuffers = std::vector<VkFramebuffer>(image_count);
		for (int i = 0; i < image_count; ++i) {
			VkImageView attachments[2]{mSwapchainImageViews[i],
									   mDepthAttachment.view};
			fb_ci.pAttachments = &attachments[0];
			fb_ci.attachmentCount = 2;
			VK_CHECK(vkCreateFramebuffer(mDevice, &fb_ci, nullptr,
										 &mFramebuffers[i]));
		}
	}
} // namespace render::vulkan
