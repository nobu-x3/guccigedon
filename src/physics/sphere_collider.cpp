#include "physics/sphere_collider.h"
#include <glm/geometric.hpp>

namespace physics {
	SphereCollider::SphereCollider(
		ArrayList<gameplay::Transform>& transform_list, s32 transform_index,
		float radius) :
		mTransformList(transform_list),
		mTransformIndex(transform_index), mRadius(radius) {}

	SphereCollider::SphereCollider(SphereCollider&& other) noexcept :
		mTransformList(other.mTransformList) {
		mRadius = other.mRadius;
		mTransformIndex = other.mTransformIndex;
		other.mRadius = 0;
		other.mTransformIndex = -1;
	}

	SphereCollider& SphereCollider::operator=(SphereCollider&& other) noexcept {
		mTransformList = other.mTransformList;
		mTransformIndex = other.mTransformIndex;
		mRadius = other.mRadius;
		other.mRadius = 0;
		other.mTransformIndex = -1;
		return *this;
	}

	IntersectData
	SphereCollider::check_collision(const SphereCollider& other) const {
		float radius_distance = mRadius + other.mRadius;
		float center_distance =
			glm::distance(mTransformList[mTransformIndex].position(),
						  other.mTransformList[other.mTransformIndex].position());
		float distance = center_distance - radius_distance;
		return IntersectData(center_distance < radius_distance, distance);
	}
} // namespace physics
