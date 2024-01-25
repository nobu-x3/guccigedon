#include <gtest/gtest.h>
#include "gameplay/transform.h"
#include "physics/aabb_collider.h"

using namespace physics;

TEST(Guccigedon_AABBCollider, AABB_AABB_intersect) {
	gameplay::Transform t_aabb1{};
	t_aabb1.position({0, 0, 0});
	gameplay::Transform t_aabb2{};
	t_aabb2.position({1, 1, 1});
	gameplay::Transform t_aabb3{};
	t_aabb3.position({1.5, 0.5, 0.5});
	gameplay::Transform t_aabb4{};
	t_aabb4.position({0.5, 0.5, -1.5});
	gameplay::Transform t_aabb5{};
	t_aabb5.position({0, 0, 0});
	ArrayList<gameplay::Transform> transforms {t_aabb1, t_aabb2, t_aabb3, t_aabb4, t_aabb5};
	physics::AABBCollider aabb1{transforms, 0, {1, 1, 1}};
	physics::AABBCollider aabb2{transforms, 1, {1, 1, 1}};
	physics::AABBCollider aabb3{transforms, 2, {1, 1, 1}};
	physics::AABBCollider aabb4{transforms, 3, {1, 1, 1}};
	physics::AABBCollider aabb5{transforms, 4, {2, 2, 2}};
	IntersectData intersect11 = aabb1.check_collision(aabb1);
	EXPECT_TRUE(intersect11.intersect);
	EXPECT_EQ(intersect11.distance, -1);
	IntersectData intersect12 = aabb1.check_collision(aabb2);
	EXPECT_FALSE(intersect12.intersect);
	EXPECT_EQ(intersect12.distance, 0);
	IntersectData intersect13 = aabb1.check_collision(aabb3);
	EXPECT_FALSE(intersect13.intersect);
	EXPECT_EQ(intersect13.distance, 0.5);
	IntersectData intersect14 = aabb1.check_collision(aabb4);
	EXPECT_FALSE(intersect14.intersect);
	EXPECT_EQ(intersect13.distance, 0.5);
	IntersectData intersect15 = aabb1.check_collision(aabb5);
	EXPECT_TRUE(intersect15.intersect);
	EXPECT_EQ(intersect15.distance, -1.5);
}
