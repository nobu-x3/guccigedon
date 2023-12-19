#include "physics/physics_engine.h"
#include <glm/vec3.hpp>
#include <gtest/gtest.h>
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
	gameplay::MovementComponent m1{{0, 0, 0}, {0, 0, 0}};
	auto index = physics.add_physics_object(0, physics::ColliderType::Sphere,
											settings, m1);
	const physics::PhysicsObject& po = physics.physics_object()[index];
	EXPECT_EQ(index, 0);
	EXPECT_EQ(po.movemevent_component_index, 0);
	EXPECT_EQ(po.transform_index, 0);
	EXPECT_FALSE(po.has_input);
	index =
		physics.add_physics_object(1, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 1);
	index =
		physics.add_physics_object(2, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 2);
	index =
		physics.add_physics_object(3, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 3);
}

TEST(Guccigedon_PhysicsEngine, simulate_uniform_motion) {
  ArrayList<core::Entity> entities{{0, -1, 0}};
	gameplay::Transform t1;
	gameplay::Transform t2;
	gameplay::Transform t3;
	ArrayList<gameplay::Transform> transforms{t1, t2, t3};
	core::Engine engine{entities, transforms};
	physics::Engine physics{&engine};
	gameplay::MovementComponent m1{{0,0,0}, {0, 0, 1}};
	gameplay::MovementComponent m2{{0,0,0}, {3, 2, 1}};
	gameplay::MovementComponent m3{{0,0,0}, {-4, 0, 1}};
	physics.add_physics_object(0, physics::ColliderType::None, {}, m1);
	physics.add_physics_object(1, physics::ColliderType::None, {}, m2);
	physics.add_physics_object(2, physics::ColliderType::None, {}, m3);
	physics.simulate(20.0f);
	EXPECT_EQ(engine.transforms()[0].position().x, 0);
	EXPECT_EQ(engine.transforms()[0].position().y, 0);
	EXPECT_EQ(engine.transforms()[0].position().z, 20);
	EXPECT_EQ(engine.transforms()[1].position().x, 60);
	EXPECT_EQ(engine.transforms()[1].position().y, 40);
	EXPECT_EQ(engine.transforms()[1].position().z, 20);
	EXPECT_EQ(engine.transforms()[2].position().x, -80);
	EXPECT_EQ(engine.transforms()[2].position().y, 0);
	EXPECT_EQ(engine.transforms()[2].position().z, 20);
}

TEST(Guccigedon_PhysicsEngine, simulate_non_uniform_motion) {
  ArrayList<core::Entity> entities{{0, -1, 0}};
	gameplay::Transform t1;
	gameplay::Transform t2;
	gameplay::Transform t3;
	ArrayList<gameplay::Transform> transforms{t1, t2, t3};
	core::Engine engine{entities, transforms};
	physics::Engine physics{&engine};
	gameplay::MovementComponent m1{{0,0,1}, {0, 0, 1}};
	gameplay::MovementComponent m2{{1,1,1}, {3, 2, 1}};
	gameplay::MovementComponent m3{{3,0,5}, {-4, 0, 1}};
	physics.add_physics_object(0, physics::ColliderType::None, {}, m1);
	physics.add_physics_object(1, physics::ColliderType::None, {}, m2);
	physics.add_physics_object(2, physics::ColliderType::None, {}, m3);
	physics.simulate(20.0f);
	EXPECT_EQ(engine.transforms()[0].position().x, 0);
	EXPECT_EQ(engine.transforms()[0].position().y, 0);
	EXPECT_EQ(engine.transforms()[0].position().z, 420);
	EXPECT_EQ(engine.transforms()[1].position().x, 460);
	EXPECT_EQ(engine.transforms()[1].position().y, 440);
	EXPECT_EQ(engine.transforms()[1].position().z, 420);
	EXPECT_EQ(engine.transforms()[2].position().x, 1120);
	EXPECT_EQ(engine.transforms()[2].position().y, 0);
	EXPECT_EQ(engine.transforms()[2].position().z, 2020);
}
