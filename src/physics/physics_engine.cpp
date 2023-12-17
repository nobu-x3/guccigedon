#include "physics/physics_engine.h"
#include "core/sapfire_engine.h"

namespace physics {

	Engine::Engine(core::Engine* core_engine) : mCoreEngine(core_engine) {}

	s32 Engine::add_physics_object(s32 transform_index, ColliderType type,
								   ColliderSettings settings,
								   bool add_movement) {
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
		mPhysicsObjects.push_back(object);
		return mPhysicsObjects.size() - 1;
	}

	void Engine::simulate(f32 delta_time) {}
} // namespace physics
