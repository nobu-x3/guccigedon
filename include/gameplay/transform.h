#pragma once

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gameplay {
	class Transform {
	public:
		const glm::mat4& transform();
		inline const glm::vec3& euler() const { return mEulerAngles; }
		inline const glm::vec3& position() const { return mPosition; }
		inline const glm::vec3& scale() const { return mScale; }

		inline Transform& scale(const glm::vec3& scale) {
			mScale = scale;
			mDirty = true;
			return *this;
		}

		inline Transform& position(const glm::vec3& pos) {
			mPosition = pos;
			mDirty = true;
			return *this;
		}

		inline Transform& rotation(const glm::vec3& rotation) {
			mEulerAngles = rotation;
			mRotation = {mEulerAngles};
			mDirty = true;
			return *this;
		}

	private:
		bool mDirty{true};
		glm::mat4x4 mTransform{1.0};
		glm::vec3 mPosition{0.0};
		glm::quat mRotation{};
		glm::vec3 mEulerAngles{0.0};
		glm::vec3 mScale{1.0};
	};
} // namespace gameplay
