#include "physics/sphere_collider.h"
#include <gtest/gtest.h>

TEST(SphereColliderTest, IntersectTest) {
	physics::SphereCollider sphere1(glm::vec3{0, 0, 0}, 1.f);
	physics::SphereCollider sphere2(glm::vec3{0, 3, 0}, 1.f);
	physics::SphereCollider sphere3(glm::vec3{2, 0, 0}, 1.f);
	physics::SphereCollider sphere4(glm::vec3{0, 0, 1}, 1.f);
	auto intersect12 = sphere1.check_collision(sphere2);
	auto intersect13 = sphere1.check_collision(sphere3);
	auto intersect14 = sphere1.check_collision(sphere4);
	EXPECT_FALSE(intersect12.intersect);
	EXPECT_FALSE(intersect13.intersect);
	EXPECT_TRUE(intersect14.intersect);
	EXPECT_EQ(intersect12.distance, 1);
	EXPECT_EQ(intersect13.distance, 0);
	EXPECT_EQ(intersect14.distance, -1);
}
