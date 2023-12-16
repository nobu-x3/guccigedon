#include "physics/sphere_collider.h"
#include <glm/geometric.hpp>

namespace physics {
	SphereCollider::SphereCollider(glm::vec3 center, float radius) noexcept :
		mCenter(center), mRadius(radius) {}

	SphereCollider::SphereCollider(SphereCollider&& other) noexcept {
		mCenter = other.mCenter;
		mRadius = other.mRadius;
		other.mRadius = 0;
		other.mCenter = {0, 0, 0};
	}

	SphereCollider& SphereCollider::operator=(SphereCollider&& other) noexcept {
		mCenter = other.mCenter;
		mRadius = other.mRadius;
		other.mRadius = 0;
		other.mCenter = {0, 0, 0};
		return *this;
	}

	IntersectData
	SphereCollider::check_collision(const SphereCollider& other) const {
		float radius_distance = mRadius + other.mRadius;
		float center_distance = glm::distance(mCenter, other.mCenter);
		float distance = center_distance - radius_distance;
		return IntersectData(center_distance < radius_distance, distance);
	}
} // namespace physics
