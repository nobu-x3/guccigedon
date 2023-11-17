#pragma once

#include "core/types.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace gameplay {
	struct MovementComponent {
		f32 speed = 0.001f;
		glm::vec3 velocity{0};

		// Helper but don't actually need it
		void update_velocity(glm::vec3 direction, const glm::mat4& rotation);
	};
} // namespace gameplay