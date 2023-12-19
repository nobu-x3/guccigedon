#pragma once

#include "core/types.h"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

namespace gameplay {
	struct MovementComponent {
        glm::vec3 acceleration{0};
		glm::vec3 velocity{0};
	};
} // namespace gameplay
