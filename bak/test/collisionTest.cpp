#include "gtest/gtest.h"

#include "bak/collision.hpp"
#include "bak/model.hpp"

#include "com/logger.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace BAK {

struct CollisionTestFixture : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Logging::LogState::SetLevel(Logging::LogLevel::Debug);
    }
};

TEST_F(CollisionTestFixture, PointInHouseClip)
{
    // Constructed from z01.json house gid entry
    // Single ClipElement with 8 ClipPoints forming an octagon (radius 800)
    ModelClip houseClip{
        .mRadius = {800, 800},
        .mFlags = 0,
        .mWalkable = false,
        .mHasVertical = false,
        .mElements = {{
            .mPoints = {
                ClipPoint{{-100, -100}, {-800, -600}},
                ClipPoint{{   0,  -75}, {-600, -800}},
                ClipPoint{{ 100, -100}, { 600, -800}},
                ClipPoint{{  75,    0}, { 800, -600}},
                ClipPoint{{ 100,  100}, { 800,  600}},
                ClipPoint{{   0,   75}, { 600,  800}},
                ClipPoint{{-100,  100}, {-600,  800}},
                ClipPoint{{ -75,    0}, {-800,  600}},
            },
            .mHeightPoint = std::nullopt,
            .mScale = 0,
            .mBaseHeight = 0
        }},
        .mName = "house"
    };

    // 1. Point at origin - should be inside (center of octagon)
    EXPECT_TRUE(PointInModelClip({0, 0}, houseClip));

    // 2. Point inside, off-center - should be inside
    EXPECT_TRUE(PointInModelClip({400, 0}, houseClip));

    // 3. Point outside bounding radius - should be outside
    EXPECT_FALSE(PointInModelClip({1000, 0}, houseClip));

    // 4. Point within radius but outside clip shape (above the diagonal edge)
    EXPECT_FALSE(PointInModelClip({700, 710}, houseClip));
}

TEST_F(CollisionTestFixture, PointInT010001ElementZero)
{
    // Element 0 from t010001 gid in z01.json (lines 30334-30383)
    // 4-point clip quadrilateral
    ModelClip t01Element0{
        .mRadius = {16000, 16000},
        .mFlags = 0,
        .mWalkable = false,
        .mHasVertical = false,
        .mElements = {{
            .mPoints = {
                ClipPoint{{-125,   0}, {6400, -16000}},
                ClipPoint{{ -50, 100}, {6400, -15000}},
                ClipPoint{{  87,   0}, {7200, -14600}},
                ClipPoint{{   0,-100}, {7200, -16000}},
            },
            .mHeightPoint = std::nullopt,
            .mScale = 0,
            .mBaseHeight = 0
        }},
        .mName = "t010001_elem0"
    };

    // 1. Near centroid of element 0's quadrilateral - should be inside
    EXPECT_TRUE(PointInModelClip({6800, -15400}, t01Element0));

    // 2. Within radius but outside element 0 - should be outside
    EXPECT_FALSE(PointInModelClip({14000, 0}, t01Element0));

    // 3. Outside bounding radius - should be outside
    EXPECT_FALSE(PointInModelClip({16100, 0}, t01Element0));
}

TEST_F(CollisionTestFixture, PointInR010407ElementZero)
{
    // Element 0 from r010407 gid in z01.json
    // 4-point clip quadrilateral with radius [16000, 9600]
    ModelClip r010407Elem0{
        .mRadius = {16000, 9600},
        .mFlags = 0,
        .mWalkable = false,
        .mHasVertical = false,
        .mElements = {{
            .mPoints = {
                ClipPoint{{   0, -125}, {16000, -8000}},
                ClipPoint{{-100,  100}, {12000, -8000}},
                ClipPoint{{   0,   75}, {13600, -6400}},
                ClipPoint{{ 100,    0}, {16000, -6400}},
            },
            .mHeightPoint = std::nullopt,
            .mScale = 0,
            .mBaseHeight = 0
        }},
        .mName = "r010407_elem0"
    };

    // 1. Near centroid of element 0's quadrilateral - should be inside
    EXPECT_TRUE(PointInModelClip({14400, -7200}, r010407Elem0));

    // 2. Within radius but outside element 0 - should be outside
    EXPECT_FALSE(PointInModelClip({16000, -1000}, r010407Elem0));

    // 3. Outside bounding radius - should be outside
    EXPECT_FALSE(PointInModelClip({16001, 0}, r010407Elem0));
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceIdentity)
{
    auto result = WorldToModelClipSpace({10, 20}, {0, 0}, 0.0f, 1.0f);
    EXPECT_FLOAT_EQ(result.x, 10.0f);
    EXPECT_FLOAT_EQ(result.y, 20.0f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceScaleOnly)
{
    auto result = WorldToModelClipSpace({10, 20}, {0, 0}, 0.0f, 2.0f);
    EXPECT_FLOAT_EQ(result.x, 5.0f);
    EXPECT_FLOAT_EQ(result.y, 10.0f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceRotation90)
{
    auto result = WorldToModelClipSpace({0, 10}, {0, 0}, glm::radians(90.0f), 1.0f);
    EXPECT_NEAR(result.x, 10.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceRotation180)
{
    auto result = WorldToModelClipSpace({10, 0}, {0, 0}, glm::radians(180.0f), 1.0f);
    EXPECT_NEAR(result.x, -10.0f, 1e-6f);
    EXPECT_NEAR(result.y, 0.0f, 1e-6f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceRotationMinus90)
{
    auto result = WorldToModelClipSpace({10, 0}, {0, 0}, glm::radians(-90.0f), 1.0f);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, 10.0f, 1e-6f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceRotationAndScale)
{
    auto result = WorldToModelClipSpace({10, 0}, {0, 0}, glm::radians(90.0f), 5.0f);
    EXPECT_NEAR(result.x, 0.0f, 1e-6f);
    EXPECT_NEAR(result.y, -2.0f, 1e-6f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceRotation45)
{
    const float sqrt2_2 = std::sqrt(2.0f) / 2.0f;
    auto result = WorldToModelClipSpace({1, 0}, {0, 0}, glm::radians(45.0f), 1.0f);
    EXPECT_NEAR(result.x, sqrt2_2, 1e-6f);
    EXPECT_NEAR(result.y, -sqrt2_2, 1e-6f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceNonZeroModelPos)
{
    auto result = WorldToModelClipSpace({30, 50}, {10, 20}, 0.0f, 1.0f);
    EXPECT_FLOAT_EQ(result.x, 20.0f);
    EXPECT_FLOAT_EQ(result.y, 30.0f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceNonZeroModelPosWithScale)
{
    auto result = WorldToModelClipSpace({40, 70}, {10, 20}, 0.0f, 2.0f);
    EXPECT_FLOAT_EQ(result.x, 15.0f);
    EXPECT_FLOAT_EQ(result.y, 25.0f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceNonZeroModelPosWithRotation)
{
    auto result = WorldToModelClipSpace({55, 55}, {50, 50}, glm::radians(90.0f), 1.0f);
    EXPECT_NEAR(result.x, 5.0f, 1e-6f);
    EXPECT_NEAR(result.y, -5.0f, 1e-6f);
}

TEST_F(CollisionTestFixture, WorldToModelClipSpaceNonZeroModelPosWithRotationAndScale)
{
    auto result = WorldToModelClipSpace({70, 55}, {50, 50}, glm::radians(90.0f), 2.0f);
    EXPECT_NEAR(result.x, 2.5f, 1e-6f);
    EXPECT_NEAR(result.y, -10.0f, 1e-6f);
}

}
