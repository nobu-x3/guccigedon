#pragma once

#include <filesystem>
#include <optional>
#include "assets/scene/gltf_importer.h"
#include "core/input.h"
#include "core/types.h"
#include "gameplay/movement_component.h"
#include "physics/aabb_collider.h"
#include "physics/plane_collider.h"
#include "physics/sphere_collider.h"

namespace core {
	class Engine;
}

namespace tinygltf {
	class Node;
	class Model;
} // namespace tinygltf

namespace physics {

	class Engine {
	public:
		Engine(core::Engine* core_engine);
		// returns index of the newly added physics object
		s32 add_physics_object(
			s32 transform_index, ColliderType type, ColliderSettings settings,
			std::optional<gameplay::MovementComponent> movement_comp = {},
			std::optional<RigidBody> rb = {});

		void handle_input_event(core::PollResult& poll_result);
		void simulate(f32 delta_time);
		void handle_collisions();
        void load_scene(const asset::GLTFImporter& scene_asset);

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
		struct Node {
			Node* parent{nullptr};
			ArrayList<Node*> children{};
			std::optional<RigidBody> rb{};
			ColliderType collider_type{ColliderType::None};
			std::optional<ColliderSettings> settings;
			~Node() {
				for (Node* child : children) {
					delete child;
				}
			}
		};

		glm::vec3 compute_force(const RigidBody& rb);

		void load_node(const tinygltf::Node* inputNode,
					   const tinygltf::Model* input, Node* parent);
        f32 calculate_separating_velocity(const CollisionContact& contact);
        void resolve_velocity(f32 duration);
        void resolve_interpenetration(f32 duration);

	private:
		core::Engine* mCoreEngine{nullptr};
		ArrayList<PhysicsObject> mPhysicsObjects{};
		ArrayList<gameplay::MovementComponent> mMovementComponents{};
		ArrayList<RigidBody> mRigidBodies{};
		ArrayList<SphereCollider> mSpheres{};
		ArrayList<AABBCollider> mAABBs{};
		ArrayList<PlaneCollider> mPlanes{};
        ArrayList<CollisionContact> mCollisions{};
	};
} // namespace physics
