#include "gameplay/camera.h"
#include <glm/ext/matrix_transform.hpp>

namespace gameplay {
	Camera::Camera(f32 fov, f32 aspect, f32 near, f32 far) :
		projection(glm::perspective(fov, aspect, near, far)) {
		projection[1][1] *= -1;
	}

	void Camera::update(f32 dt) {
		if (input.mouse_state.RMB) {
			auto euler = transform.euler();
			euler.y += input.mouse_delta_x;
			euler.x += input.mouse_delta_y;
			transform.rotation(euler);
		}
		input.reset();
	}

} // namespace gameplay
