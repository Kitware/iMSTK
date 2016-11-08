#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "imstkMath.h"
#include "imstkSphere.h"
using namespace imstk;

class imstkSphereTest : public ::testing::Test
{
protected:
    Sphere m_sphere;
};

TEST_F(imstkSphereTest, SetGetRadius)
{
    m_sphere.setRadius(2);
    EXPECT_EQ(m_sphere.getRadius(), 2);

    m_sphere.setRadius(0.003);
    EXPECT_EQ(m_sphere.getRadius(), 0.003);

    m_sphere.setRadius(400000000);
    EXPECT_EQ(m_sphere.getRadius(), 400000000);

    m_sphere.setRadius(0);
    EXPECT_GT(m_sphere.getRadius(), 0);

    m_sphere.setRadius(-5);
    EXPECT_GT(m_sphere.getRadius(), 0);
}

TEST_F(imstkSphereTest, GetVolume)
{
    m_sphere.setRadius(2);
    EXPECT_EQ(m_sphere.getVolume(), 8*0.75*PI);

    m_sphere.setRadius(0.003);
    EXPECT_EQ(m_sphere.getVolume(), 0.003*0.003*0.003*0.75*PI);

    double r = 400000000;
    m_sphere.setRadius(400000000);
    EXPECT_EQ(m_sphere.getVolume(), r*r*r*0.75*PI);
}

int imstkSphereTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
