#include "render/vulkan/device.h"
#include <vulkan/vulkan_core.h>
#include "render/vulkan/builders.h"
#include "render/vulkan/types.h"

namespace render::vulkan {
	Device::Device(vkb::Instance vkb_inst, VkSurfaceKHR surface) :
		mLifetime(ObjectLifetime::OWNED) {
		vkb::PhysicalDeviceSelector selector{vkb_inst};
		vkb::PhysicalDevice vkb_phys_dev = selector.set_minimum_version(1, 1)
											   .set_surface(surface)
											   .select()
											   .value();
		VkPhysicalDeviceShaderDrawParametersFeatures shader_dram_param_feat{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
			nullptr, VK_TRUE};
		vkb::DeviceBuilder dev_builder{vkb_phys_dev};
		vkb::Device vkb_dev =
			dev_builder.add_pNext(&shader_dram_param_feat).build().value();
		mDevice = vkb_dev.device;
		mPhysicalDevice = vkb_phys_dev.physical_device;
		mPhysicalDeviceProperties = vkb_dev.physical_device.properties;
		mGraphicsQueue = vkb_dev.get_queue(vkb::QueueType::graphics).value();
		mGraphicsQueueFamily =
			vkb_dev.get_queue_index(vkb::QueueType::graphics).value();
		// initialize the memory allocator
		VmaAllocatorCreateInfo allocator_ci = {};
		allocator_ci.physicalDevice = mPhysicalDevice;
		allocator_ci.device = mDevice;
		allocator_ci.instance = vkb_inst.instance;
		vmaCreateAllocator(&allocator_ci, &mAllocator);
	}

	Device::Device(Device& other) {
		mDevice = other.mDevice;
		mPhysicalDevice = other.mPhysicalDevice;
		mPhysicalDeviceProperties = other.mPhysicalDeviceProperties;
		mGraphicsQueue = other.mGraphicsQueue;
		mGraphicsQueueFamily = other.mGraphicsQueueFamily;
		mLifetime = ObjectLifetime::OWNED;
		mAllocator = other.mAllocator;
		other.mLifetime = ObjectLifetime::TEMP;
	}

	Device::Device(Device&& device) noexcept {
		mDevice = device.mDevice;
		mPhysicalDevice = device.mPhysicalDevice;
		mPhysicalDeviceProperties = device.mPhysicalDeviceProperties;
		mGraphicsQueue = device.mGraphicsQueue;
		mGraphicsQueueFamily = device.mGraphicsQueueFamily;
		mAllocator = device.mAllocator;
		mLifetime = ObjectLifetime::OWNED;
		device.mLifetime = ObjectLifetime::TEMP;
	}

	Device& Device::operator=(Device& other) {
		mDevice = other.mDevice;
		mPhysicalDevice = other.mPhysicalDevice;
		mPhysicalDeviceProperties = other.mPhysicalDeviceProperties;
		mGraphicsQueue = other.mGraphicsQueue;
		mGraphicsQueueFamily = other.mGraphicsQueueFamily;
		mLifetime = ObjectLifetime::OWNED;
		mAllocator = other.mAllocator;
		other.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Device& Device::operator=(Device&& other) noexcept {
		mDevice = other.mDevice;
		mPhysicalDevice = other.mPhysicalDevice;
		mPhysicalDeviceProperties = other.mPhysicalDeviceProperties;
		mGraphicsQueue = other.mGraphicsQueue;
		mGraphicsQueueFamily = other.mGraphicsQueueFamily;
		mLifetime = ObjectLifetime::OWNED;
		mAllocator = other.mAllocator;
		other.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Device::~Device() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (mAllocator) {
			vmaDestroyAllocator(mAllocator);
		}
		if (mDevice) {
			vkDestroyDevice(mDevice, nullptr);
		}
	}
	void Device::submit_queue(VkCommandBuffer buf, VkSemaphore wait_semaphore,
							  VkSemaphore signal_semaphore, VkFence fence,
							  VkPipelineStageFlags wait_flags) {

		// waiting on present_semaphore which is signaled when swapchain is
		// ready. signal render_semaphore when finished rendering.
		VkSubmitInfo submit = vkbuild::submit_info(&buf);
		submit.pWaitDstStageMask = &wait_flags;
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &wait_semaphore;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &signal_semaphore;
		// render fence blocks until commands finish executing
		VK_CHECK(vkQueueSubmit(mGraphicsQueue, 1, &submit, fence));
	}

	void Device::present(VkSwapchainKHR swapchain, VkSemaphore wait_semaphore,
						 u32 image_index, std::function<void(void)> resized_callback) {
		// waiting on rendering to finish, then presenting
		VkPresentInfoKHR present_info = vkbuild::present_info();
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &wait_semaphore;
		present_info.pImageIndices = &image_index;
		VkResult res = vkQueuePresentKHR(mGraphicsQueue, &present_info);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
			resized_callback();
		} else if (res != VK_SUCCESS) {
			core::Logger::Error("Cannot present queue. %d", res);
		}
	}
} // namespace render::vulkan
