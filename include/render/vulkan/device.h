#pragma once

#include "VkBootstrap.h"
#include "render/vulkan/types.h"
#include <vulkan/vulkan_core.h>

namespace render::vulkan {

	class Device {
	private:
		VkPhysicalDevice mPhysicalDevice{};
		VkPhysicalDeviceProperties mPhysicalDeviceProperties{};
		VkDevice mDevice{};
		VmaAllocator mAllocator{};
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};
		VkQueue mGraphicsQueue{};
		u32 mGraphicsQueueFamily{};

	public:
		Device() = default;
		Device(vkb::Instance, VkSurfaceKHR);
		Device(Device& other);
		Device(Device&& device) noexcept;
		Device& operator=(Device& other);
		Device& operator=(Device&& other) noexcept;
		~Device();

		inline void wait_idle() { vkDeviceWaitIdle(mDevice); }

		inline VkPhysicalDevice physical_device() const {
			return mPhysicalDevice;
		}

		inline VkPhysicalDeviceProperties physical_device_properties() const {
			return mPhysicalDeviceProperties;
		}

		inline VkDevice logical_device() const { return mDevice; }

		inline VkQueue graphics_queue() const { return mGraphicsQueue; }

		inline u32 graphics_queue_family() const {
			return mGraphicsQueueFamily;
		}

		inline VmaAllocator allocator() { return mAllocator; }
	};
} // namespace render::vulkan
