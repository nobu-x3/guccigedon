#pragma once

#include "core/types.h"

namespace gameplay {
	struct MovementComponent {
		f32 speed;
		glm::vec3 direction{0};
		glm::vec3 velocity{0};

		glm::vec3 delta(f32 delta_time);
	};
} // namespace gameplay