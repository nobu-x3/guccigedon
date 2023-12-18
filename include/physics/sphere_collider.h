#pragma once
#include <glm/vec3.hpp>
#include "gameplay/transform.h"
#include "core/types.h"
#include "intersect_data.h"

namespace physics {
	class SphereCollider {
	public:
		SphereCollider(ArrayList<gameplay::Transform>& transform_list,
					   s32 transform_index, float radius);
		SphereCollider(const SphereCollider&) = delete;
		SphereCollider& operator=(const SphereCollider&) = delete;
		SphereCollider(SphereCollider&& other) noexcept;
		SphereCollider& operator=(SphereCollider&&) noexcept;
		IntersectData check_collision(const SphereCollider& other) const;
		inline float radius() const { return mRadius; }
		inline const glm::vec3& center() const {
			return mTransformList[mTransformIndex].position();
		}

	private:
		float mRadius;
		s32 mTransformIndex;
		ArrayList<gameplay::Transform>& mTransformList;
	};
} // namespace physics
