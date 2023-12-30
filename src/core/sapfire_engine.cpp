#include "core/sapfire_engine.h"

namespace core {

	Engine::Engine() { mPhysics = std::make_unique<physics::Engine>(this); }

	void Engine::load_scene(std::filesystem::path scene_path) {
		mRenderer =
			std::make_unique<render::vulkan::VulkanRenderer>(scene_path);
		mPhysics->load_scene(scene_path);
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
				// mPhysics->handle_input_event(poll_result);
				mRenderer->handle_input_event(poll_result);
			} while (poll_result.result != 0);
			mPhysics->simulate(0.016f);
			mRenderer->draw();
		}
	}
} // namespace core
