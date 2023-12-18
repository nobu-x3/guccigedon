#include "physics/physics_engine.h"
#include <gtest/gtest.h>
#include <glm/vec3.hpp>
#include "core/sapfire_engine.h"
#include "gameplay/transform.h"

TEST(Guccigedon_PhysicsEngine, add_physics_object) {
	ArrayList<core::Entity> entities{
		{0, -1, 0}, {1, -1, 1}, {2, -1, 2}, {3, -1, 3}};
	gameplay::Transform t1;
	t1.position({0, 0, 0});
	gameplay::Transform t2;
	t2.position({0, 3, 0});
	gameplay::Transform t3;
	t3.position({2, 0, 0});
	gameplay::Transform t4;
	t4.position({0, 0, 1});
	ArrayList<gameplay::Transform> transforms{t1, t2, t3, t4};
	core::Engine engine{entities, transforms};
	EXPECT_EQ(engine.transforms().size(), 4);
	EXPECT_EQ(engine.entity_count(), 4);
	physics::Engine physics{&engine};
	physics::ColliderSettings settings;
	settings.radius = 1.f;
	gameplay::MovementComponent m1 {1.f, {0,0,0}};
	auto index = physics.add_physics_object(0, physics::ColliderType::Sphere, settings, m1);
	const physics::PhysicsObject& po = physics.physics_object()[index];
	EXPECT_EQ(index, 0);
	EXPECT_EQ(po.movemevent_component_index, 0);
	EXPECT_EQ(po.transform_index, 0);
	EXPECT_FALSE(po.has_input);
	index = physics.add_physics_object(1, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 1);
	index = physics.add_physics_object(2, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 2);
	index = physics.add_physics_object(3, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 3);
}

TEST(Guccigedon_PhysicsEngine, simulate_simple) {
	ArrayList<core::Entity> entities{
		{0, -1, 0}};
	gameplay::Transform t1;
	EXPECT_EQ(t1.forward().z, -1);
	ArrayList<gameplay::Transform> transforms{t1};
	core::Engine engine{entities, transforms};
	physics::Engine physics{&engine};
	gameplay::MovementComponent m1 {1.f, {0,0,1}};
	s32 index = physics.add_physics_object(0, physics::ColliderType::None, {}, m1);
	physics.simulate(20.0f);
	EXPECT_EQ(engine.transforms()[0].position().z, 20);
}
