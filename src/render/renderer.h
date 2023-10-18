#pragma once
#include "vulkan_types.h"

struct SDL_Window;

namespace render {
	class VulkanRenderer {

	public:
		int _curr_frame{0};
		VkExtent2D _window_extent{800, 600};
		SDL_Window *_window{nullptr};

	public:
		VulkanRenderer();
		~VulkanRenderer();
		void draw();
		void run();
	};
} // namespace render
