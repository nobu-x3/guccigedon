#pragma once

#include <glm/vec3.hpp>
#include "core/types.h"
#include "physics/physics_types.h"
#include "physics/sphere_collider.h"

namespace physics {
	class PlaneCollider {
	public:
		PlaneCollider(glm::vec3 normal, float distance);
		PlaneCollider(const PlaneCollider&) = default;
		PlaneCollider& operator=(const PlaneCollider&) = default;
		PlaneCollider(PlaneCollider&&) = default;
		PlaneCollider& operator=(PlaneCollider&&) = default;
		PlaneCollider normalized() const;
		IntersectData check_collision(const SphereCollider& sphere);
		inline const glm::vec3& normal() const { return mNormal; }
		inline const f32 distance() const { return mDistance; }
		inline void normal(const glm::vec3& other) { mNormal = other; }
		inline void distance(const float distance) { mDistance = distance; }

	private:
		glm::vec3 mNormal;
		// distance from world origin along mNormal
		f32 mDistance;
	};
} // namespace physics
