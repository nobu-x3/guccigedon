#include "render/vulkan/swapchain.h"
#include <VkBootstrap.h>
#include "core/logger.h"
#include "render/vulkan/builders.h"
#include "render/vulkan/renderer.h"
#include "render/vulkan/types.h"
#include "vulkan/vulkan_core.h"

namespace render::vulkan {

	Swapchain::Swapchain(VmaAllocator allocator, Device& device,
						 Surface& surface, VkExtent2D window_extent,
						 VkRenderPass renderpass) :
		mDevice(device.logical_device()),
		mPhysicalDevice(device.physical_device()), mWindowExtent(window_extent),
		mSurface(surface.surface()), mAllocator(allocator),
		mLifetime(ObjectLifetime::OWNED), mSwapchainDescription(surface.surface(), device.physical_device()) {
		u32 img_count{};
		img_count = mSwapchainDescription.capabilities.minImageCount + 1;
		if (mSwapchainDescription.capabilities.maxImageCount > 0 &&
			img_count > mSwapchainDescription.capabilities.maxImageCount) {
			img_count = mSwapchainDescription.capabilities.maxImageCount;
		}
		if (img_count > MAXIMUM_FRAMES_IN_FLIGHT) {
			img_count = MAXIMUM_FRAMES_IN_FLIGHT;
		}
		VkSwapchainCreateInfoKHR swapchain_ci{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr};
		// swapchain_ci.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		swapchain_ci.surface = surface.surface();
		swapchain_ci.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		swapchain_ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchain_ci.minImageCount = img_count;
		swapchain_ci.imageExtent = window_extent;
		swapchain_ci.imageArrayLayers = 1;
		swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_ci.preTransform =
			mSwapchainDescription.capabilities.currentTransform;
		swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_ci.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		swapchain_ci.clipped = 1;
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_ci.queueFamilyIndexCount = 0;
		swapchain_ci.pQueueFamilyIndices = nullptr;
		VK_CHECK(
			vkCreateSwapchainKHR(mDevice, &swapchain_ci, nullptr, &mSwapchain));
		img_count = 0;
		VK_CHECK(
			vkGetSwapchainImagesKHR(mDevice, mSwapchain, &img_count, nullptr));
		mSwapchainImages.resize(img_count);
		VK_CHECK(vkGetSwapchainImagesKHR(mDevice, mSwapchain, &img_count,
										 mSwapchainImages.data()));
		mSwapchainImageViews.resize(img_count);
		for (int i = 0; i < img_count; ++i) {
			VkImageViewCreateInfo view_ci{
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr};
			view_ci.image = mSwapchainImages[i];
			view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_ci.format = VK_FORMAT_B8G8R8A8_SRGB;
			view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			VK_CHECK(vkCreateImageView(mDevice, &view_ci, nullptr,
									   &mSwapchainImageViews[i]));
		}
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
		mWindowExtent(window_extent), mPhysicalDevice(device.physical_device()),
		mSurface(surface.surface()), mLifetime(ObjectLifetime::OWNED),
		mAllocator(allocator),
		mSwapchainDescription(surface.surface(), device.physical_device()) {
		u32 img_count{};
		img_count = mSwapchainDescription.capabilities.minImageCount + 1;
		if (mSwapchainDescription.capabilities.maxImageCount > 0 &&
			img_count > mSwapchainDescription.capabilities.maxImageCount) {
			img_count = mSwapchainDescription.capabilities.maxImageCount;
		}
		if (img_count > MAXIMUM_FRAMES_IN_FLIGHT) {
			img_count = MAXIMUM_FRAMES_IN_FLIGHT;
		}
		VkSwapchainCreateInfoKHR swapchain_ci{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr};
		// swapchain_ci.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		swapchain_ci.surface = surface.surface();
		swapchain_ci.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		swapchain_ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchain_ci.minImageCount = img_count;
		swapchain_ci.imageExtent = window_extent;
		swapchain_ci.imageArrayLayers = 1;
		swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_ci.preTransform =
			mSwapchainDescription.capabilities.currentTransform;
		swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_ci.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		swapchain_ci.clipped = 1;
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_ci.queueFamilyIndexCount = 0;
		swapchain_ci.pQueueFamilyIndices = nullptr;
		VK_CHECK(
			vkCreateSwapchainKHR(mDevice, &swapchain_ci, nullptr, &mSwapchain));
		img_count = 0;
		VK_CHECK(
			vkGetSwapchainImagesKHR(mDevice, mSwapchain, &img_count, nullptr));
		mSwapchainImages.resize(img_count);
		VK_CHECK(vkGetSwapchainImagesKHR(mDevice, mSwapchain, &img_count,
										 mSwapchainImages.data()));
		mSwapchainImageViews.resize(img_count);
		for (int i = 0; i < img_count; ++i) {
			VkImageViewCreateInfo view_ci{
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr};
			view_ci.image = mSwapchainImages[i];
			view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_ci.format = VK_FORMAT_B8G8R8A8_SRGB;
			view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			VK_CHECK(vkCreateImageView(mDevice, &view_ci, nullptr,
									   &mSwapchainImageViews[i]));
		}
		mSwapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
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

	void Swapchain::rebuild(u32 width, u32 height, VkRenderPass renderpass) {
		vkDeviceWaitIdle(mDevice);
		for (int i = 0; i < mFramebuffers.size(); ++i) {
			if (mFramebuffers[i])
				vkDestroyFramebuffer(mDevice, mFramebuffers[i], nullptr);
			if (mSwapchainImageViews[i])
				vkDestroyImageView(mDevice, mSwapchainImageViews[i], nullptr);
		}
		if (mSwapchain) {
			vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
		}
		mWindowExtent = {width, height};
		mSwapchainDescription = {mSurface, mPhysicalDevice};
		u32 img_count{};
		img_count = mSwapchainDescription.capabilities.minImageCount + 1;
		if (mSwapchainDescription.capabilities.maxImageCount > 0 &&
			img_count > mSwapchainDescription.capabilities.maxImageCount) {
			img_count = mSwapchainDescription.capabilities.maxImageCount;
		}
		if (img_count > MAXIMUM_FRAMES_IN_FLIGHT) {
			img_count = MAXIMUM_FRAMES_IN_FLIGHT;
		}
		VkSwapchainCreateInfoKHR swapchain_ci{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr};
		// swapchain_ci.imageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
		swapchain_ci.surface = mSurface;
		swapchain_ci.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		swapchain_ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchain_ci.minImageCount = img_count;
		swapchain_ci.imageExtent = mWindowExtent;
		swapchain_ci.imageArrayLayers = 1;
		swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_ci.preTransform =
			mSwapchainDescription.capabilities.currentTransform;
		swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_ci.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		swapchain_ci.clipped = 1;
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_ci.queueFamilyIndexCount = 0;
		swapchain_ci.pQueueFamilyIndices = nullptr;
		VK_CHECK(
			vkCreateSwapchainKHR(mDevice, &swapchain_ci, nullptr, &mSwapchain));
		img_count = 0;
		VK_CHECK(
			vkGetSwapchainImagesKHR(mDevice, mSwapchain, &img_count, nullptr));
		mSwapchainImages.resize(img_count);
		VK_CHECK(vkGetSwapchainImagesKHR(mDevice, mSwapchain, &img_count,
										 mSwapchainImages.data()));
		mSwapchainImageViews.resize(img_count);
		for (int i = 0; i < img_count; ++i) {
			VkImageViewCreateInfo view_ci{
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr};
			view_ci.image = mSwapchainImages[i];
			view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_ci.format = VK_FORMAT_B8G8R8A8_SRGB;
			view_ci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
			VK_CHECK(vkCreateImageView(mDevice, &view_ci, nullptr,
									   &mSwapchainImageViews[i]));
		}
		VkExtent3D depthImageExtent = {width, height, 1};
		VmaAllocationCreateInfo imgAllocCi{};
		imgAllocCi.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		imgAllocCi.requiredFlags =
			VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mDepthAttachment = {
			mAllocator, mDevice,
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

	SwapchainDescription::SwapchainDescription(VkSurfaceKHR surface,
											   VkPhysicalDevice device) {
		VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
														   &capabilities));
	}

	SwapchainDescription::SwapchainDescription(
		const SwapchainDescription& other) {
		capabilities = other.capabilities;
	}

	SwapchainDescription&
	SwapchainDescription::operator=(const SwapchainDescription& other) {
		capabilities = other.capabilities;
		return *this;
	}

	SwapchainDescription::SwapchainDescription(
		SwapchainDescription&& other) noexcept {
		capabilities = other.capabilities;
	}

	SwapchainDescription& SwapchainDescription::operator=(
		const SwapchainDescription&& other) noexcept {
		capabilities = other.capabilities;
		return *this;
	}
} // namespace render::vulkan
