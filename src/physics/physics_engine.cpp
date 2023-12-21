#include "physics/physics_engine.h"
#include "core/sapfire_engine.h"
#include <tiny_gltf.h>

namespace physics {

	Engine::Engine(core::Engine* core_engine) : mCoreEngine(core_engine) {}

	void Engine::load_scene(std::filesystem::path scene_path) {
		tinygltf::Model input;
		tinygltf::TinyGLTF context;
		std::string error, warning;
		bool loaded =
			context.LoadASCIIFromFile(&input, &error, &warning, scene_path.string());
		if (loaded) {
			const tinygltf::Scene& scene = input.scenes[0];
			for (int i = 0; i < scene.nodes.size(); ++i) {
				const tinygltf::Node node = input.nodes[scene.nodes[i]];
				load_node(&node, &input, nullptr);
			}
		} else {
			throw std::exception();
		}
    }

    static s32 transform_index {0};
	void Engine::load_node(const tinygltf::Node* inNode,
						   const tinygltf::Model* in, Node* parent) {
		const tinygltf::Node& inputNode = *inNode;
		const tinygltf::Model& input = *in;
		Node* node = new Node{};
		node->parent = parent;
        if(inputNode.extras.Has("PhysicsObject")){
            PhysicsObject po {transform_index};
            // TODO: import all this stuyff
        }
        ++transform_index;


    }

	s32 Engine::add_physics_object(
		s32 transform_index, ColliderType type, ColliderSettings settings,
		std::optional<gameplay::MovementComponent> movement_comp,
		std::optional<RigidBody> rb) {
		PhysicsObject object{transform_index};
		object.collider_type = type;
		switch (type) {
		case ColliderType::Sphere:
			mSpheres.emplace_back(mCoreEngine->transforms(), transform_index,
								  settings.radius);
			object.collider_component_index = mSpheres.size() - 1;
			break;
		case ColliderType::AABB:
			mAABBs.emplace_back(mCoreEngine->transforms(), transform_index,
								settings.size);
			object.collider_component_index = mAABBs.size() - 1;
			break;
		case ColliderType::Plane:
			mPlanes.emplace_back(settings.normal, settings.distance);
			object.collider_component_index = mPlanes.size() - 1;
			break;
		default:
			break;
		}
		object.movemevent_component_index = -1;
		if (movement_comp.has_value()) {
			mMovementComponents.push_back(movement_comp.value());
			object.movemevent_component_index = mMovementComponents.size() - 1;
		}
		object.rigidbody_component_index = -1;
		if (rb.has_value()) {
			mRigidBodies.push_back(rb.value());
			object.rigidbody_component_index = mRigidBodies.size() - 1;
		}
		mPhysicsObjects.push_back(object);
		return mPhysicsObjects.size() - 1;
	}

	// TODO: this is supposed to be pretty complicated alas
	glm::vec3 Engine::compute_force(const RigidBody& rb) {
		return glm::vec3{0, rb.mass * -9.81 * rb.gravity_factor, 0};
	}

	void Engine::simulate(f32 delta_time) {
		for (auto& po : mPhysicsObjects) {
			if (po.movemevent_component_index == -1)
				continue;
			if (po.movemevent_component_index >= mMovementComponents.size()) {
				core::Logger::Error(
					"Physics objects array contains an out of date physics "
					"object with movement component index {}, index out of "
					"range, array size {}",
					po.movemevent_component_index, mMovementComponents.size());
				continue;
			}
			if (po.transform_index >= mCoreEngine->transforms().size()) {
				core::Logger::Error(
					"Physics objects array contains an out of date physics "
					"object with transform index {}, index out of "
					"range, array size {}",
					po.transform_index, mCoreEngine->transforms().size());
				continue;
			}
			auto& transform = mCoreEngine->transforms()[po.transform_index];
			auto& movement = mMovementComponents[po.movemevent_component_index];
			if (po.rigidbody_component_index >= 0) {
				auto& rb = mRigidBodies[po.rigidbody_component_index];
				auto force = compute_force(rb);
				movement.acceleration = glm::vec3{
					force.x / rb.mass, force.y / rb.mass, force.z / rb.mass};
			}
			movement.velocity += movement.acceleration * delta_time;
			auto position = transform.position();
			position += movement.velocity * delta_time;
			transform.position(position);
		}
	}

	void Engine::handle_collisions() {}
} // namespace physics
