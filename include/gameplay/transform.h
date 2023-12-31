#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/types.h"

namespace gameplay {
	class Transform {
	public:
		const glm::mat4&
		calculate_transform(const ArrayList<Transform>& transforms);
		const glm::mat4& transform() const { return mTransform; }
		inline const glm::vec3& euler() const { return mEulerAngles; }
		inline const glm::vec3& position() const { return mPosition; }
		inline const glm::vec3& scale() const { return mScale; }
		inline const glm::vec3& right() const { return mRight; }
		inline const glm::vec3& forward() const { return mForward; }
		inline const glm::vec3& up() const { return mUp; }

		inline Transform& scale(const glm::vec3& scale) {
			mScale = scale;
			mDirty = true;
			return *this;
		}

		inline Transform& position(const glm::vec3& pos) {
			mPosition = pos;
			return *this;
		}

		Transform& rotation(const glm::vec3& rotation);

		Transform& rotation(const glm::quat& rotation);

		inline const glm::mat4& rotation_matrix() const {
			return mRotationMatrix;
		}

		inline void parent_index(s32 ind) {
			mParentIndex = ind;
			mDirty = true;
		}

		inline s32 parent_index() const { return mParentIndex; }

	private:
		glm::mat4x4 mTransform{1.0};
		glm::vec3 mPosition{0.0};
		glm::quat mRotation{};
		glm::vec3 mEulerAngles{0.0, 0.0, 0.0};
		glm::vec3 mScale{1.0, 1.0, 1.0};
		glm::vec3 mRight{1.0, 0.0, 0.0};
		glm::vec3 mForward{0.0, 0.0, -1.0};
		glm::vec3 mUp{0.0, 1.0, 0.0};
		glm::mat4x4 mRotationMatrix{1};
		s32 mParentIndex{-1};
		bool mDirty{true};
	};
} // namespace gameplay
