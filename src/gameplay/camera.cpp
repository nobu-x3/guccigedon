#include "gameplay/camera.h"
#include <glm/ext/matrix_transform.hpp>

namespace gameplay {
	Camera::Camera(f32 fov, f32 aspect, f32 near, f32 far) :
		projection(glm::perspective(fov, aspect, near, far)) {
		projection[1][1] *= -1;
	}

} // namespace gameplay
