#include "core/sapfire_engine.h"

namespace core {

	void Engine::load_scene(std::filesystem::path scene_path) {
		mRenderer =
			std::make_unique<render::vulkan::VulkanRenderer>(scene_path);
	}

	void Engine::run() {
		bool quit = false;
		while (!quit) {
			PollResult poll_result{};
			do {
				poll_result = InputSystem::poll_events();
				if (poll_result.event.type == SDL_QUIT) {
					quit = true;
				}
				mRenderer->handle_input_event(poll_result);
			} while (poll_result.result != 0);
			mRenderer->draw();
		}
	}
} // namespace core