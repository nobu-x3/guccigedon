#include "physics/plane_collider.h"

namespace physics {
	PlaneCollider::PlaneCollider(glm::vec3 normal, float distance) :
		mNormal(normal), mDistance(distance) {}

	PlaneCollider PlaneCollider::normalized() const {
		float magnitude = glm::length(mNormal);
		return PlaneCollider(mNormal / magnitude, mDistance / magnitude);
	}

	IntersectData PlaneCollider::check_collision(const SphereCollider& sphere) {
		f32 distance_from_center = static_cast<f32>(
			std::fabs(glm::dot(sphere.center() + mDistance, mNormal)));
		f32 distance_from_sphere = distance_from_center - sphere.radius();
		return IntersectData(distance_from_sphere < 0, distance_from_sphere);
	}
} // namespace physics
