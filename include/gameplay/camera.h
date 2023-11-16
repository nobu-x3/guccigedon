#pragma once

#include "core/types.h"
#include "gameplay/transform.h"

namespace gameplay {

	struct Camera {
		Camera() = default;
		Camera(f32 fov, f32 aspect, f32 near, f32 far);

		Transform transform{};
		glm::mat4x4 projection{1.0};

		inline glm::mat4x4 view() {
			return glm::lookAt(transform.position(),
							   transform.position() + transform.forward(),
							   transform.up());
		}

		glm::mat4x4 view_proj() { return projection * view(); }
	};
} // namespace gameplay
