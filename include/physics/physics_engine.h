#pragma once

#include <filesystem>
#include <optional>
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
		float mass = 1;
		float gravity_factor = 1;
	};

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
		void load_scene(std::filesystem::path scene_path);

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

	private:
		core::Engine* mCoreEngine{nullptr};
		ArrayList<PhysicsObject> mPhysicsObjects{};
		ArrayList<gameplay::MovementComponent> mMovementComponents{};
		ArrayList<RigidBody> mRigidBodies{};
		ArrayList<SphereCollider> mSpheres{};
		ArrayList<AABBCollider> mAABBs{};
		ArrayList<PlaneCollider> mPlanes{};
	};
} // namespace physics
