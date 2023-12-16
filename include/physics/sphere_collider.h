#pragma once
#include <glm/vec3.hpp>
#include "gameplay/transform.h"
#include "intersect_data.h"

namespace physics {
	class SphereCollider {
	public:
	  SphereCollider(gameplay::Transform* transform, float radius) noexcept;
		SphereCollider(const SphereCollider&) = delete;
		SphereCollider& operator=(const SphereCollider&) = delete;
		SphereCollider(SphereCollider&& other) noexcept;
		SphereCollider& operator=(SphereCollider&&) noexcept;
		IntersectData check_collision(const SphereCollider& other) const;
		inline float radius() const { return mRadius; }
		inline const glm::vec3& center() const {
			return mTransform->position();
		}

	private:
		float mRadius;
		gameplay::Transform* mTransform;
	};
} // namespace physics
