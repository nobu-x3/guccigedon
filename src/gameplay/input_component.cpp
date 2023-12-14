#include "gameplay/input_component.h"
#include <glm/common.hpp>

namespace gameplay {

	void InputComponent::process_input_event(SDL_Event* ev) {
		if (ev->type == SDL_KEYDOWN) {
			switch (ev->key.keysym.sym) {
			case SDLK_UP:
			case SDLK_w:
				input_axis.x += 1.f;
				break;
			case SDLK_DOWN:
			case SDLK_s:
				input_axis.x -= 1.f;
				break;
			case SDLK_LEFT:
			case SDLK_a:
				input_axis.y -= 1.f;
				break;
			case SDLK_RIGHT:
			case SDLK_d:
				input_axis.y += 1.f;
				break;
			case SDLK_q:
				input_axis.z -= 1.f;
				break;
			case SDLK_e:
				input_axis.z += 1.f;
				break;
			}
		} else if (ev->type == SDL_KEYUP) {
			switch (ev->key.keysym.sym) {
			case SDLK_UP:
			case SDLK_w:
				input_axis.x -= 1.f;
				break;
			case SDLK_DOWN:
			case SDLK_s:
				input_axis.x += 1.f;
				break;
			case SDLK_LEFT:
			case SDLK_a:
				input_axis.y += 1.f;
				break;
			case SDLK_RIGHT:
			case SDLK_d:
				input_axis.y -= 1.f;
				break;
			case SDLK_q:
				input_axis.z += 1.f;
				break;
			case SDLK_e:
				input_axis.z -= 1.f;
				break;
			}
		} else if (ev->type == SDL_MOUSEMOTION) {
			mouse_delta_y = ev->motion.yrel * sens_v;
			mouse_delta_x = ev->motion.xrel * sens_h;
		}
		input_axis =
			glm::clamp(input_axis, {-1.0, -1.0, -1.0}, {1.0, 1.0, 1.0});
		mouse_state = core::InputSystem::mouse_state();
	}

	void InputComponent::reset_mouse_state() {
		mouse_delta_x = 0.f;
		mouse_delta_y = 0.f;
		mouse_state = {};
	}

	void InputComponent::reset_input_axis() { input_axis = {0.f, 0.f, 0.f}; }
} // namespace gameplay
