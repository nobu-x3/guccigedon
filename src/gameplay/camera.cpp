#include "gameplay/camera.h"
#include <core/logger.h>
#include <glm/ext/matrix_transform.hpp>

namespace gameplay {
	Camera::Camera(f32 fov, f32 aspect, f32 near, f32 far) :
		projection(glm::perspective(fov, aspect, near_plane, far_plane)),
		fov(fov), aspect(aspect), near_plane(near), far_plane(far) {
		projection[1][1] *= -1;
	}

	void Camera::build_projection() {
		projection = glm::perspective(fov, aspect, near_plane, far_plane);
		projection[1][1] *= -1;
	}

	void Camera::update(f32 dt) {
		auto euler = transform.euler();
		euler.y += input.mouse_delta_x;
		euler.x += input.mouse_delta_y;
		transform.rotation(euler);
		movement.velocity = -input.input_axis.x * transform.forward() +
			input.input_axis.y * transform.right() +
			input.input_axis.z * transform.up();
		movement.velocity *= dt * movement.speed;
		auto position = transform.position();
		position += movement.velocity;
		transform.position(position);
		input.reset();
	}

} // namespace gameplay
