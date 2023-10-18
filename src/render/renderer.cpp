#include "renderer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <cstdlib>
#include "../core/logger.h"

namespace render {

	VulkanRenderer::VulkanRenderer() {
		SDL_Init(SDL_INIT_VIDEO);
		_window = SDL_CreateWindow("Guccigedon", SDL_WINDOWPOS_CENTERED,
								   SDL_WINDOWPOS_CENTERED, _window_extent.width,
								   _window_extent.height,
								   (SDL_WindowFlags)(SDL_WINDOW_VULKAN));
		if (!_window) {
			core::Logger::Fatal("Failed to create a window. %s",
								SDL_GetError());
			exit(-1);
		}
	}

	VulkanRenderer::~VulkanRenderer() { SDL_DestroyWindow(_window); }

	void VulkanRenderer::draw() {}

	void VulkanRenderer::run() {
		SDL_Event e;
		bool quit = false;
		while (!quit) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_QUIT)
					quit = true;
			}
			draw();
		}
	}
} // namespace render
