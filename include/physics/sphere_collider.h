#pragma once
#include <glm/vec3.hpp>
#include "intersect_data.h"

namespace physics {
	class SphereCollider {
	public:
		SphereCollider(glm::vec3 center, float radius) noexcept;
		SphereCollider(const SphereCollider&) = delete;
		SphereCollider& operator=(const SphereCollider&) = delete;
		SphereCollider(SphereCollider&& other) noexcept;
		SphereCollider& operator=(SphereCollider&&) noexcept;
		IntersectData check_collision(const SphereCollider& other) const;
		inline float radius() const { return mRadius; }
		inline glm::vec3 center() const { return mCenter; }

	private:
		float mRadius;
		glm::vec3 mCenter;
	};
} // namespace physics
