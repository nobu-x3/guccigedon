#pragma once

#include "VkBootstrap.h"
#include "render/vulkan/types.h"
#include <vulkan/vulkan_core.h>
namespace render::vulkan {

	class Instance {
	private:
		VkInstance mInstance{};
		VkDebugUtilsMessengerEXT fpDebugMsger{};
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};

	public:
		Instance() = default;
		Instance(vkb::Instance&);
		Instance(Instance& instance);
		Instance(Instance&& instance) noexcept;
		Instance& operator=(Instance& instance);
		Instance& operator=(Instance&& instance) noexcept;
		~Instance();
		inline VkInstance handle() const { return mInstance; }
		inline bool operator==(Instance& inst) const {
			return mInstance == inst.mInstance;
		}
		inline ObjectLifetime lifetime() const { return mLifetime; }
	};
} // namespace render::vulkan
