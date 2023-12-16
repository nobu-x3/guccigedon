#pragma once

#include <glm/vec3.hpp>
#include "gameplay/transform.h"
#include "intersect_data.h"

namespace physics {
	class AABBCollider {
	public:
		AABBCollider(gameplay::Transform* transform, const glm::vec3& size);
		AABBCollider(const AABBCollider&) = delete;
		AABBCollider& operator=(const AABBCollider&) = delete;
		AABBCollider(AABBCollider&&) noexcept;
		AABBCollider& operator=(AABBCollider&&) noexcept;
		IntersectData check_collision(const AABBCollider& other) const;
		inline const glm::vec3& size() const { return mSize; }

	private:
		gameplay::Transform* mTransform;
		glm::vec3 mSize;
	};

} // namespace physics
