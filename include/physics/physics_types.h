#pragma once
#include "core/types.h"
#include "glm/ext/vector_float3.hpp"

namespace physics {
	enum class ColliderType : u8 { None, Sphere, AABB, Plane, MAX };

	struct PhysicsObject {
		s32 transform_index;
		s32 collider_component_index;
		s32 movemevent_component_index;
		s32 rigidbody_component_index;
		ColliderType collider_type;
		bool has_input{false};
	};

	union ColliderSettings {
		glm::vec3 size;
		glm::vec3 normal;
		f32 radius;
		f32 distance;
	};

	struct RigidBody {
		f32 inverse_mass = 1;
		f32 gravity_factor = 1;
		f32 damping = 1;
	};

	struct CollisionContact {
		const PhysicsObject* objects[2];
		f32 restitution;
		glm::vec3 contact_normal;
	};

	struct IntersectData {
		bool intersect;
		float distance;
	};
} // namespace physics
