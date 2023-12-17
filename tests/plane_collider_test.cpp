#include "physics/plane_collider.h"
#include <gtest/gtest.h>
#include "gameplay/transform.h"
#include "physics/sphere_collider.h"

TEST(PlaneCollider, Plane_Sphere_intersect) {
	physics::PlaneCollider plane{{0, 1, 0}, 0};
	gameplay::Transform t1;
	t1.position({0, 0, 0});
	physics::SphereCollider sphere1(&t1, 1.f);
	gameplay::Transform t2;
	t2.position({0, 3, 0});
	physics::SphereCollider sphere2(&t2, 1.f);
	gameplay::Transform t3;
	t3.position({2, 0, 0});
	physics::SphereCollider sphere3(&t3, 1.f);
	gameplay::Transform t4;
	t4.position({0, 0, 1});
	physics::SphereCollider sphere4(&t4, 1.f);
	auto intersect11 = plane.check_collision(sphere1);
	auto intersect12 = plane.check_collision(sphere2);
	auto intersect13 = plane.check_collision(sphere3);
	auto intersect14 = plane.check_collision(sphere4);
	EXPECT_TRUE(intersect11.intersect);
	EXPECT_EQ(intersect11.distance, -1);
	EXPECT_FALSE(intersect12.intersect);
	EXPECT_EQ(intersect12.distance, 2);
	EXPECT_TRUE(intersect13.intersect);
	EXPECT_EQ(intersect13.distance, -1);
	EXPECT_TRUE(intersect14.intersect);
	EXPECT_EQ(intersect14.distance, -1);
}
