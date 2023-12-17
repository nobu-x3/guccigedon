#pragma once

#include "core/types.h"
#include "gameplay/movement_component.h"
#include "physics/aabb_collider.h"
#include "physics/plane_collider.h"
#include "physics/sphere_collider.h"

namespace core {
	class Engine;
}

namespace physics {
	enum class ColliderType : u8 { None, Sphere, AABB, Plane, MAX };

	struct PhysicsObject {
		s32 transform_index;
		s32 collider_component_index;
		s32 movemevent_component_index;
		ColliderType collider_type;
	};

	union ColliderSettings {
		glm::vec3 size;
		glm::vec3 normal;
		f32 radius;
		f32 distance;
	};

	class Engine {
	public:
		Engine(core::Engine* core_engine);
		// returns index of the newly added physics object
		s32 add_physics_object(s32 transform_index, ColliderType type,
							   ColliderSettings settings, bool add_movement = false);

		void simulate(f32 delta_time);

		inline const ArrayList<PhysicsObject>& physics_object() const {
			return mPhysicsObjects;
		}
		inline const ArrayList<gameplay::MovementComponent>&
		movement_components() const {
			return mMovementComponents;
		}
		inline const ArrayList<SphereCollider>& sphere_colliders() const {
			return mSpheres;
		}
		inline const ArrayList<AABBCollider>& aabb_colliders() const {
			return mAABBs;
		}
		inline const ArrayList<PlaneCollider>& plane_colliders() const {
			return mPlanes;
		}

	private:
		core::Engine* mCoreEngine{nullptr};
		ArrayList<PhysicsObject> mPhysicsObjects{};
		ArrayList<gameplay::MovementComponent> mMovementComponents{};
		ArrayList<SphereCollider> mSpheres{};
		ArrayList<AABBCollider> mAABBs{};
		ArrayList<PlaneCollider> mPlanes{};
	};
} // namespace physics
