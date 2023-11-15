#pragma once

#include <concepts>
#include <glm/fwd.hpp>

namespace gameplay {

    template<typename T>
        concept WorldObject = requires(T t){
            {t.transform} -> std::same_as<glm::mat4>;
        };
}
