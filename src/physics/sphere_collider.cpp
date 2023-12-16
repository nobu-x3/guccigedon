#include "physics/sphere_collider.h"
#include <glm/geometric.hpp>

namespace physics {
	SphereCollider::SphereCollider(gameplay::Transform* transform,
								   float radius) noexcept :
		mTransform(transform),
		mRadius(radius) {}

	SphereCollider::SphereCollider(SphereCollider&& other) noexcept {
		mTransform = other.mTransform;
		mRadius = other.mRadius;
		other.mRadius = 0;
		other.mTransform = nullptr;
	}

	SphereCollider& SphereCollider::operator=(SphereCollider&& other) noexcept {
		mTransform = other.mTransform;
		mRadius = other.mRadius;
		other.mRadius = 0;
		other.mTransform = nullptr;
		return *this;
	}

	IntersectData
	SphereCollider::check_collision(const SphereCollider& other) const {
		float radius_distance = mRadius + other.mRadius;
		float center_distance =
			glm::distance(mTransform->position(), other.mTransform->position());
		float distance = center_distance - radius_distance;
		return IntersectData(center_distance < radius_distance, distance);
	}
} // namespace physics
