#include "render/vulkan/instance.h"
#include "VkBootstrap.h"
#include "core/logger.h"
#include "render/vulkan/vulkan_types.h"

namespace render::vulkan {

	Instance::Instance(vkb::Instance& vkb_inst) :
		mInstance(vkb_inst.instance), fpDebugMsger(vkb_inst.debug_messenger),
		mLifetime(ObjectLifetime::OWNED) {}

	Instance::Instance(Instance& instance) {
		mInstance = instance.mInstance;
		fpDebugMsger = instance.fpDebugMsger;
		mLifetime = ObjectLifetime::TEMP;
	}

	Instance::Instance(Instance&& instance) noexcept {
		mInstance = instance.mInstance;
		fpDebugMsger = instance.fpDebugMsger;
		mLifetime = ObjectLifetime::OWNED;
		instance.mLifetime = ObjectLifetime::TEMP;
	}

	Instance& Instance::operator=(Instance& instance) {
		mInstance = instance.mInstance;
		fpDebugMsger = instance.fpDebugMsger;
		mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Instance& Instance::operator=(Instance&& instance) noexcept {
		mInstance = instance.mInstance;
		fpDebugMsger = instance.fpDebugMsger;
		mLifetime = ObjectLifetime::OWNED;
		instance.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Instance::~Instance() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (mInstance) {
			if (fpDebugMsger) {
				vkb::destroy_debug_utils_messenger(mInstance, fpDebugMsger);
			}
			vkDestroyInstance(mInstance, nullptr);
		}
	}
} // namespace render::vulkan
