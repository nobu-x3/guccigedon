#pragma once

#include "core/types.h"
#include "gameplay/transform.h"

namespace gameplay {

	struct Camera {
		Camera() = default;
		Camera(f32 fov, f32 aspect, f32 near, f32 far);

		Transform transform{};
		glm::mat4x4 projection{1.0};

		inline glm::mat4x4 view() const {
			glm::vec3 pos = transform.position();
			pos *= -1;
			return glm::translate(glm::mat4(1.f), pos);
		}

		glm::mat4x4 view_proj() const { return projection * view(); }
	};
} // namespace gameplay
