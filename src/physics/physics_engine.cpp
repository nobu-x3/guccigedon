#include "physics/physics_engine.h"
#include <cmath>
#include <tiny_gltf.h>
#include "core/sapfire_engine.h"

namespace physics {

	Engine::Engine(core::Engine* core_engine) : mCoreEngine(core_engine) {}

	static s32 transform_index{0};
	void Engine::load_scene(const asset::GLTFImporter& scene_asset) {
		transform_index = 0;
		for (auto& node_id : scene_asset.scene->nodes) {
			const tinygltf::Node node = scene_asset.input->nodes[node_id];
			load_node(&node, scene_asset.input, nullptr);
		}
	}

	void Engine::load_node(const tinygltf::Node* inNode,
						   const tinygltf::Model* in, Node* parent) {
		const tinygltf::Node& inputNode = *inNode;
		const tinygltf::Model& input = *in;
		Node* node = new Node{};
		node->parent = parent;
		if (inputNode.extras.Has("Physics Object")) {
			PhysicsObject po{transform_index};
			ColliderType collider_type{};
			ColliderSettings settings{};
			// TODO: import all this stuyff
			const auto& collider_string =
				inputNode.extras.Get("Collider").Get<std::string>();
			if (collider_string.compare("AABB") == 0) {
				collider_type = ColliderType::AABB;
				if (inputNode.scale.size() > 0) {
					settings.size = {inputNode.scale[0], inputNode.scale[1],
									 inputNode.scale[2]};
				} else {
					settings.size = {1, 1, 1};
				}
			} else if (collider_string.compare("Sphere") == 0) {
				collider_type = ColliderType::Sphere;
				if (inputNode.scale.size() > 0) {
					settings.radius = inputNode.scale[0];
				} else {
					settings.radius = 1.f;
				}
			} else if (collider_string.compare("Plane") == 0) {
				collider_type = ColliderType::Plane;
				// TODO: plane stuff
			}
			std::optional<RigidBody> rb{};
			if (inputNode.extras.Get("Rigidbody").Get<bool>()) {
				RigidBody rb_val{
					static_cast<float>(
						inputNode.extras.Get("Mass").Get<double>()),
					static_cast<float>(
						inputNode.extras.Get("Gravity Factor").Get<double>())};
				rb = rb_val;
			}
			add_physics_object(
				transform_index, collider_type, settings,
				rb.has_value() ? gameplay::MovementComponent{}
							   : std::optional<gameplay::MovementComponent>{},
				rb);
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
		return glm::vec3{0, rb.inverse_mass * -9.81 * rb.gravity_factor, 0};
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
			f32 damping = 1;
			if (po.rigidbody_component_index >= 0) {
				auto& rb = mRigidBodies[po.rigidbody_component_index];
				if (rb.inverse_mass > 0.f) {
					auto force = compute_force(rb);
					movement.acceleration = force * rb.inverse_mass;
					damping = std::pow(rb.damping, delta_time);
				}
			}
			movement.velocity += movement.acceleration * delta_time;
			movement.velocity *= damping;
			auto position = transform.position();
			position += movement.velocity * delta_time;
			transform.position(position);
		}
	}

	void Engine::handle_collisions() {}

	f32 Engine::calculate_separating_velocity(const CollisionContact& contact) {
		glm::vec3 relative_velocity{0};
		if (contact.objects[0] &&
			contact.objects[0]->movemevent_component_index >= 0) {
			relative_velocity =
				mMovementComponents[contact.objects[0]
										->movemevent_component_index]
					.velocity;
		}
		if (contact.objects[1] &&
			contact.objects[1]->movemevent_component_index >= 0) {
			relative_velocity -=
				mMovementComponents[contact.objects[1]
										->movemevent_component_index]
					.velocity;
		}
		return glm::dot(relative_velocity, contact.contact_normal);
	}

	void Engine::resolve_velocity(f32 duration) {
		for (const auto& contact : mCollisions) {
			f32 separating_velocity = calculate_separating_velocity(contact);
			// contact either separating or stationary, so no impulse required
			if (separating_velocity > 0) {
				return;
			}
			f32 new_sep_velocity = -separating_velocity * contact.restitution;
			f32 delta_vel = new_sep_velocity - separating_velocity;
			f32 total_inverse_mass = 0;
			if (contact.objects[0] &&
				contact.objects[0]->rigidbody_component_index >= 0) {
				total_inverse_mass +=
					mRigidBodies[contact.objects[0]->rigidbody_component_index]
						.inverse_mass;
			}
			if (contact.objects[1] &&
				contact.objects[1]->rigidbody_component_index >= 0) {
				total_inverse_mass +=
					mRigidBodies[contact.objects[1]->rigidbody_component_index]
						.inverse_mass;
			}
			if (total_inverse_mass <= 0) {
				return;
			}
			f32 impulse = delta_vel / total_inverse_mass;
			glm::vec3 impulse_per_mass = contact.contact_normal * impulse;
			// apply impulses
			if (contact.objects[0] &&
				contact.objects[0]->movemevent_component_index >= 0 &&
				contact.objects[0]->rigidbody_component_index >= 0) {
				const auto& inv_mass =
					mRigidBodies[contact.objects[0]->rigidbody_component_index]
						.inverse_mass;
				mMovementComponents[contact.objects[0]
										->movemevent_component_index]
					.velocity += impulse_per_mass * inv_mass;
			}
			if (contact.objects[1] &&
				contact.objects[1]->movemevent_component_index >= 0 &&
				contact.objects[1]->rigidbody_component_index >= 0) {
				const auto& inv_mass =
					mRigidBodies[contact.objects[1]->rigidbody_component_index]
						.inverse_mass;
				mMovementComponents[contact.objects[1]
										->movemevent_component_index]
					.velocity += impulse_per_mass * -inv_mass;
			}
		}
	}
} // namespace physics
