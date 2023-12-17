#pragma once
#include <glm/ext/vector_float3.hpp>
#include "core/input.h"

namespace gameplay {
	struct InputComponent {
		glm::vec3 input_axis;
		float sens_v = 0.002f;
		float sens_h = 0.002f;
		float mouse_delta_x = 0.f;
		float mouse_delta_y = 0.f;
		core::MouseState mouse_state{};
		void process_input_event(SDL_Event* e);
		void reset_mouse_state();
		void reset_input_axis();
	};
} // namespace gameplay
