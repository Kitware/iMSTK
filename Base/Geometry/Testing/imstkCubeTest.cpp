#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "imstkMath.h"
#include "imstkCube.h"
using namespace imstk;

class imstkCubeTest : public ::testing::Test
{
protected:
    Cube m_cube;
};

TEST_F(imstkCubeTest, SetGetWidth)
{
    m_cube.setWidth(2);
    EXPECT_EQ(m_cube.getWidth(), 2);

    m_cube.setWidth(0.003);
    EXPECT_EQ(m_cube.getWidth(), 0.003);

    m_cube.setWidth(400000000);
    EXPECT_EQ(m_cube.getWidth(), 400000000);

    m_cube.setWidth(0);
    EXPECT_GT(m_cube.getWidth(), 0);

    m_cube.setWidth(-5);
    EXPECT_GT(m_cube.getWidth(), 0);
}

TEST_F(imstkCubeTest, GetVolume)
{
    m_cube.setWidth(2);
    EXPECT_EQ(m_cube.getVolume(), 8);

    m_cube.setWidth(0.003);
    EXPECT_EQ(m_cube.getVolume(), 0.003*0.003*0.003);

    double w = 400000000;
    m_cube.setWidth(400000000);
    EXPECT_EQ(m_cube.getVolume(), w*w*w);
}

int imstkCubeTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
