#include "render/vulkan/surface.h"
#include "render/vulkan/vulkan_types.h"

namespace render::vulkan {

	Surface::Surface(SDL_Window* window, Instance& instance) :
		mLifetime(ObjectLifetime::OWNED), mInstance(instance) {
		SDL_Vulkan_CreateSurface(window, instance.instance(), &mSurface);
	}

	Surface::Surface(Surface& surface) : mLifetime(ObjectLifetime::OWNED) {
		mSurface = surface.mSurface;
		surface.mLifetime = ObjectLifetime::TEMP;
	}

	Surface::Surface(Surface&& surface) noexcept :
		mLifetime(ObjectLifetime::OWNED) {
		mSurface = surface.mSurface;
		surface.mLifetime = ObjectLifetime::TEMP;
	}

	Surface& Surface::operator=(Surface& surface) {
		mSurface = surface.mSurface;
		mLifetime = ObjectLifetime::OWNED;
		surface.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Surface& Surface::operator=(Surface&& surface) noexcept {
		mSurface = surface.mSurface;
		mLifetime = ObjectLifetime::OWNED;
		surface.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

    Surface::~Surface(){
        if(mLifetime == ObjectLifetime::TEMP) return;
		if (mSurface) {
			vkDestroySurfaceKHR(mInstance.instance(), mSurface, nullptr);
		}

    }
} // namespace render::vulkan
