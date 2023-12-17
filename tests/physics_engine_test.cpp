#include "physics/physics_engine.h"
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
	auto index = physics.add_physics_object(0, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 0);
	index = physics.add_physics_object(1, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 1);
	index = physics.add_physics_object(2, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 2);
	index = physics.add_physics_object(3, physics::ColliderType::Sphere, settings);
	EXPECT_EQ(index, 3);
}
