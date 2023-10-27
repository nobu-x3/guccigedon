#pragma once

#include <SDL_vulkan.h>
#include "render/vulkan/instance.h"
#include "render/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"
namespace render::vulkan {

	class Surface {
	public:
		Surface() = default;
		Surface(SDL_Window* window, Instance& instance);
		Surface(Surface& surface);
		Surface(Surface&& surface) noexcept;
		Surface& operator=(Surface& surface);
		Surface& operator=(Surface&& surface) noexcept;
		~Surface();

		inline VkSurfaceKHR surface() const { return mSurface; }

	private:
		VkSurfaceKHR mSurface{};
		ObjectLifetime mLifetime{ObjectLifetime::TEMP};
        Instance mInstance;
	};
} // namespace render::vulkan
