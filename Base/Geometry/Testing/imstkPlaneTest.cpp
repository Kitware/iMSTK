#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "imstkMath.h"
#include "imstkPlane.h"
using namespace imstk;

class imstkPlaneTest : public ::testing::Test
{
protected:
    Plane m_plane;
};

TEST_F(imstkPlaneTest, SetGetWidth)
{
    m_plane.setWidth(2);
    EXPECT_EQ(m_plane.getWidth(), 2);

    m_plane.setWidth(0.003);
    EXPECT_EQ(m_plane.getWidth(), 0.003);

    m_plane.setWidth(400000000);
    EXPECT_EQ(m_plane.getWidth(), 400000000);

    m_plane.setWidth(0);
    EXPECT_GT(m_plane.getWidth(), 0);

    m_plane.setWidth(-5);
    EXPECT_GT(m_plane.getWidth(), 0);
}

TEST_F(imstkPlaneTest, SetGetNormal)
{
    m_plane.setNormal(Vec3d(0.2, -0.3, 0.9));
    EXPECT_EQ(m_plane.getNormal(), Vec3d(0.2, -0.3, 0.9));

    m_plane.setNormal(Vec3d(0.003, -0.001, 0.002));
    EXPECT_EQ(m_plane.getNormal(), Vec3d(0.003, -0.001, 0.002));

    m_plane.setNormal(Vec3d(400000000, -500000000, 600000000));
    EXPECT_EQ(m_plane.getNormal(), Vec3d(400000000, -500000000, 600000000));

    m_plane.setNormal(WORLD_ORIGIN);
    EXPECT_NE(m_plane.getNormal(), WORLD_ORIGIN);
}

TEST_F(imstkPlaneTest, GetVolume)
{
    EXPECT_EQ(m_plane.getVolume(), 0);
}

int imstkPlaneTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
