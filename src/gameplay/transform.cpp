#include "gameplay/transform.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace gameplay {
	const glm::mat4& Transform::transform() {
		if (mDirty) {
			glm::mat4 scale = glm::scale(mScale);
			glm::mat4 rot = glm::toMat4(mRotation);
			glm::mat4 transl = glm::translate(glm::mat4(1), mPosition);
			mTransform = transl * rot * scale;
		}
		return mTransform;
	}

	Transform& Transform::rotation(const glm::vec3& rotation) {
		mEulerAngles = rotation;
		mRotation = glm::quat(rotation);
		glm::mat4 yaw_rot =
			glm::rotate(glm::mat4{1}, mEulerAngles.y, {0, -1, 0});
		glm::mat4 roll_rot =
			glm::rotate(glm::mat4{yaw_rot}, mEulerAngles.z, {0, 0, -1});
		mRotationMatrix =
			glm::rotate(glm::mat4{roll_rot}, mEulerAngles.x, {-1, 0, 0});
		mForward = {0, 0, 1};
		mRight = {1, 0, 0};
		mForward = mRotationMatrix * glm::vec4(mForward, 0.f);
		mRight = mRotationMatrix * glm::vec4(mRight, 0.f);
		mUp = glm::normalize(glm::cross(mRight, mForward));
		mDirty = true;
		return *this;
	}

} // namespace gameplay
