#include <gtest/gtest.h>
#include "gameplay/transform.h"
#include "physics/aabb.h"

TEST(AABBCollider, AABB_AABB_intersect) {
	gameplay::Transform t_aabb1{};
	t_aabb1.position({0, 0, 0});
	physics::AABBComponent aabb1{&t_aabb1, {1, 1, 1}};

	gameplay::Transform t_aabb2{};
	t_aabb2.position({1, 1, 1});
	physics::AABBComponent aabb2{&t_aabb2, {1, 1, 1}};

	gameplay::Transform t_aabb3{};
	t_aabb3.position({1.5, 0.5, 0.5});
	physics::AABBComponent aabb3{&t_aabb3, {1, 1, 1}};

	gameplay::Transform t_aabb4{};
	t_aabb4.position({0.5, 0.5, -1.5});
	physics::AABBComponent aabb4{&t_aabb4, {1, 1, 1}};

	gameplay::Transform t_aabb5{};
	t_aabb5.position({0, 0, 0});
	physics::AABBComponent aabb5{&t_aabb5, {2, 2, 2}};

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
