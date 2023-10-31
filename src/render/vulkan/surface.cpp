#include "render/vulkan/surface.h"
#include "render/vulkan/types.h"

namespace render::vulkan {

	Surface::Surface(SDL_Window* window, VkInstance instance) :
		mLifetime(ObjectLifetime::OWNED), mInstance(instance) {

		SDL_Vulkan_CreateSurface(window, mInstance, &mSurface);
	}

	Surface::Surface(Surface& surface) : mLifetime(ObjectLifetime::OWNED),mInstance(surface.mInstance) {
		mSurface = surface.mSurface;
		surface.mLifetime = ObjectLifetime::TEMP;
	}

	Surface::Surface(Surface&& surface) noexcept :
		mLifetime(ObjectLifetime::OWNED) {
		mSurface = surface.mSurface;
        mInstance = surface.mInstance;
		surface.mLifetime = ObjectLifetime::TEMP;
	}

	Surface& Surface::operator=(Surface& surface) {
		mSurface = surface.mSurface;
		mLifetime = ObjectLifetime::OWNED;
        mInstance = surface.mInstance;
		surface.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Surface& Surface::operator=(Surface&& surface) noexcept {
		mSurface = surface.mSurface;
		mLifetime = ObjectLifetime::OWNED;
        mInstance = surface.mInstance;
		surface.mLifetime = ObjectLifetime::TEMP;
		return *this;
	}

	Surface::~Surface() {
		if (mLifetime == ObjectLifetime::TEMP)
			return;
		if (mSurface) {
			vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		}
	}
} // namespace render::vulkan
