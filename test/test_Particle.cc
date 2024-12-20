#include <m2/physics/Particle.h>
#include <gtest/gtest.h>
#include <m2/Log.h>

using namespace m2;

TEST(Particle, ApplyForce) {
	Particle p1{PhysicsPrimitive{2}, // Mass
			Vec{PhysicsPrimitive{1}, PhysicsPrimitive{1}}, // Initial position
			Vec<PhysicsPrimitive>{}, // Gravitational acceleration
			PhysicsPrimitive{1}}; // Damping
	p1.ApplyForce(Vec{PhysicsPrimitive{-10}, PhysicsPrimitive{20}});

	EXPECT_EQ(p1.Acceleration().X(), PhysicsPrimitive{-5});
	EXPECT_EQ(p1.Acceleration().Y(), PhysicsPrimitive{10});
}

TEST(Particle, IntegrateAndClearAcceleration) {
	Particle p1{PhysicsPrimitive{2}, // Mass
			Vec{PhysicsPrimitive{5}, PhysicsPrimitive{5}}, // Initial position
			Vec<PhysicsPrimitive>{}, // Gravitational acceleration
			PhysicsPrimitive{1}}; // Damping
	p1.ApplyForce(Vec{PhysicsPrimitive{10}, PhysicsPrimitive{-10}});

	p1.Integrate();
	EXPECT_TRUE(p1.Position().X().IsEqual(PhysicsPrimitive{5.0025f}, PhysicsPrimitive{0.00001f}));
	EXPECT_TRUE(p1.Position().Y().IsEqual(PhysicsPrimitive{4.9975f}, PhysicsPrimitive{0.00001f}));
	EXPECT_TRUE(p1.Velocity().X().IsEqual(PhysicsPrimitive{0.05f}, PhysicsPrimitive{0.00001f}));
	EXPECT_TRUE(p1.Velocity().Y().IsEqual(PhysicsPrimitive{-0.05f}, PhysicsPrimitive{0.00001f}));

	p1.Integrate();
	EXPECT_TRUE(p1.Position().X().IsEqual(PhysicsPrimitive{5.003f}, PhysicsPrimitive{0.00001f}));
	EXPECT_TRUE(p1.Position().Y().IsEqual(PhysicsPrimitive{4.997f}, PhysicsPrimitive{0.00001f}));
	EXPECT_TRUE(p1.Velocity().X().IsEqual(PhysicsPrimitive{0.05f}, PhysicsPrimitive{0.00001f}));
	EXPECT_TRUE(p1.Velocity().Y().IsEqual(PhysicsPrimitive{-0.05f}, PhysicsPrimitive{0.00001f}));
}
