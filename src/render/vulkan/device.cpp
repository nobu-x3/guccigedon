#include "render/vulkan/device.h"
#include "render/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"

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
	}

	Device::Device(Device& other) {
		mDevice = other.mDevice;
		mPhysicalDevice = other.mPhysicalDevice;
		mPhysicalDeviceProperties = other.mPhysicalDeviceProperties;
		mGraphicsQueue = other.mGraphicsQueue;
		mGraphicsQueueFamily = other.mGraphicsQueueFamily;
		mLifetime = ObjectLifetime::OWNED;
		other.mLifetime = ObjectLifetime::TEMP;
	}

	Device::Device(Device&& device) noexcept {
		mDevice = device.mDevice;
		mPhysicalDevice = device.mPhysicalDevice;
		mPhysicalDeviceProperties = device.mPhysicalDeviceProperties;
		mGraphicsQueue = device.mGraphicsQueue;
		mGraphicsQueueFamily = device.mGraphicsQueueFamily;
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
		other.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Device::~Device() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (mDevice) {
			vkDestroyDevice(mDevice, nullptr);
		}
	}
} // namespace render::vulkan
