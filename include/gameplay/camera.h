#pragma once

#include "core/types.h"
#include "gameplay/input_component.h"
#include "gameplay/movement_component.h"
#include "gameplay/transform.h"

namespace gameplay {

	struct Camera {
		Camera() = default;
		Camera(f32 fov, f32 aspect, f32 near_plane, f32 far_plane);

		void build_projection();

		Transform transform{};
		InputComponent input{};
		MovementComponent movement{{0,0,0}, {0,0,0}};
		glm::mat4x4 projection{1.0};

		f32 fov;
		f32 aspect;
		f32 near_plane;
		f32 far_plane;

		inline glm::mat4x4 view() {
			glm::mat4 view =
				glm::translate(glm::mat4{1}, transform.position()) *
				transform.rotation_matrix();
			view = glm::inverse(view);
			return view;
		}

		glm::mat4x4 view_proj() { return projection * view(); }

		void update(f32 dt);
	};
} // namespace gameplay
