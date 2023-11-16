#include "gameplay/transform.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>

namespace gameplay {
	const glm::mat4& Transform::transform() {
		if (mDirty) {
			glm::mat4 scale = glm::scale(mScale);
			glm::mat4 rot = glm::toMat4(mRotation);
			glm::mat4 transl = glm::translate(glm::mat4(), mPosition);
			mTransform = transl * rot * scale;
			mDirty = false;
		}
		return mTransform;
	}
} // namespace gameplay
