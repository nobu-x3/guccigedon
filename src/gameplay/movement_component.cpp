#include "gameplay/movement_component.h"

namespace gameplay {
	void MovementComponent::update_velocity(glm::vec3 direction, const glm::mat4& rotation) {
		glm::vec3 forward = {0, 0, speed};
		glm::vec3 right = {speed, 0, 0};
		glm::vec3 up = {0, speed, 0};
		forward = rotation * glm::vec4(forward, 0.f);
		right = rotation * glm::vec4(right, 0.f);
		velocity =
			-direction.x * forward + direction.y * right + direction.z * up;
	}
} // namespace gameplay