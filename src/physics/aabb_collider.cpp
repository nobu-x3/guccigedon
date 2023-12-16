#include "physics/aabb_collider.h"
#include <glm/gtx/component_wise.hpp>

namespace physics {
	AABBCollider::AABBCollider(gameplay::Transform* transform,
								 const glm::vec3& size) :
		mTransform(transform),
		mSize(size) {}

	AABBCollider::AABBCollider(AABBCollider&& other) noexcept {
		mTransform = other.mTransform;
		mSize = other.mSize;
		other.mSize = {0, 0, 0};
		other.mTransform = nullptr;
	}

	AABBCollider& AABBCollider::operator=(AABBCollider&& other) noexcept {
		mTransform = other.mTransform;
		mSize = other.mSize;
		other.mSize = {0, 0, 0};
		other.mTransform = nullptr;
		return *this;
	}

	IntersectData
	AABBCollider::check_collision(const AABBCollider& other) const {
		const glm::vec3 this_pos = mTransform->position();
		const glm::vec3 other_pos = other.mTransform->position();
		const glm::vec3 max_extent_a = {this_pos.x + (mSize.x * 0.5f),
										this_pos.y + (mSize.y * 0.5f),
										this_pos.z + (mSize.z * 0.5f)};
		const glm::vec3 max_extent_b = {other_pos.x + (other.mSize.x * 0.5f),
										other_pos.y + (other.mSize.y * 0.5f),
										other_pos.z + (other.mSize.z * 0.5f)};
		const glm::vec3 min_extent_a = {this_pos.x - (mSize.x * 0.5f),
										this_pos.y - (mSize.y * 0.5f),
										this_pos.z - (mSize.z * 0.5f)};
		const glm::vec3 min_extent_b = {other_pos.x - (other.mSize.x * 0.5f),
										other_pos.y - (other.mSize.y * 0.5f),
										other_pos.z - (other.mSize.z * 0.5f)};
		const glm::vec3 distance1 = min_extent_b - max_extent_a;
		const glm::vec3 distance2 = min_extent_a - max_extent_b;
		const glm::vec3 distance = glm::max(distance1, distance2);
		const float max_val = glm::compMax(distance);
		return IntersectData(max_val < 0, max_val);
	}
} // namespace physics
