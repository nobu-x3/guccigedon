#pragma once

#include <glm/vec3.hpp>
#include "gameplay/transform.h"
#include "intersect_data.h"

namespace physics {
	class AABBComponent {
	public:
		AABBComponent(gameplay::Transform* transform, const glm::vec3& size);
		AABBComponent(const AABBComponent&) = delete;
		AABBComponent& operator=(const AABBComponent&) = delete;
		AABBComponent(AABBComponent&&) noexcept;
		AABBComponent& operator=(AABBComponent&&) noexcept;
		IntersectData check_collision(const AABBComponent& other) const;
		inline const glm::vec3& size() const { return mSize; }

	private:
		gameplay::Transform* mTransform;
		glm::vec3 mSize;
	};

} // namespace physics
