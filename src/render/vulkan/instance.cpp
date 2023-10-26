#include "render/vulkan/instance.h"
#include "VkBootstrap.h"
#include "render/vulkan/vulkan_types.h"

namespace render::vulkan {

	Instance::Instance(ObjectLifetime lifetime) : mLifetime(lifetime) {
		// all this vkb stuff is nice and all but eventually we will make
		// something more robust manually. There's a ton of unsafe stuff here.
		vkb::InstanceBuilder builder;
		vkb::Instance vkb_inst = builder.set_app_name("Guccigedon")
									 .request_validation_layers(true)
									 .use_default_debug_messenger()
									 .require_api_version(1, 1, 0)
									 .build()
									 .value();
		mInstance = vkb_inst.instance;
		fpDebugMsger = vkb_inst.debug_messenger;
	}

	Instance::Instance(vkb::Instance& vkb_inst) :
		mInstance(vkb_inst.instance), fpDebugMsger(vkb_inst.debug_messenger),
		mLifetime(ObjectLifetime::OWNED) {}

	Instance::Instance(Instance& instance) {
		mInstance = instance.mInstance;
		fpDebugMsger = instance.fpDebugMsger;
		mLifetime = ObjectLifetime::OWNED;
		instance.mLifetime = ObjectLifetime::TEMP;
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
		mLifetime = ObjectLifetime::OWNED;
		instance.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Instance& Instance::operator=(Instance&& instance) noexcept {
		mInstance = instance.mInstance;
		fpDebugMsger = instance.fpDebugMsger;
		mLifetime = ObjectLifetime::OWNED;
		instance.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

    Instance::~Instance(){
        if(mLifetime == ObjectLifetime::TEMP) return;
		if (mInstance) {
			if (fpDebugMsger) {
				vkb::destroy_debug_utils_messenger(mInstance, fpDebugMsger);
			}
			vkDestroyInstance(mInstance, nullptr);
		}
    }
} // namespace render::vulkan
