#include "physics/physics_engine.h"
#include "core/sapfire_engine.h"

namespace physics {

	Engine::Engine(core::Engine* core_engine) : mCoreEngine(core_engine) {}

	s32 Engine::add_physics_object(
		s32 transform_index, ColliderType type, ColliderSettings settings,
		std::optional<gameplay::MovementComponent> movement_comp) {
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
		mPhysicsObjects.push_back(object);
		return mPhysicsObjects.size() - 1;
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
			// @TODO: fix later, add gravity, bla
			// movement.direction = transform.forward();
			auto position = transform.position();
			position += movement.direction * delta_time * movement.speed;;
			transform.position(position);
		}
	}
} // namespace physics
