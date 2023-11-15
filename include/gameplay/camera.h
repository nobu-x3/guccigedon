#pragma once

#include <glm/ext/matrix_transform.hpp>
#include "core/types.h"

namespace gameplay {
    struct Camera {
        Camera() = default;
        Camera(f32 fov, f32 aspect, f32 near, f32 far);
        glm::mat4x4 transform{1.0};
        glm::mat4x4 projection{1.0};
        glm::mat4x4 view{1.0};
    };
}
