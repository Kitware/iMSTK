/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "gtest/gtest.h"
#include <g3log/g3log.hpp>

#include "imstkLoopThreadObject.h"

namespace imstk
{
///
/// \brief TODO
///
class LoopThreadObjectMock : public LoopThreadObject
{
public:
    LoopThreadObjectMock() : LoopThreadObject("modulemock") { }
    ~LoopThreadObjectMock() override = default;

    bool m_init    = false;
    bool m_run     = false;
    bool m_cleanup = false;

protected:
    inline void initThread() override { m_init = true; }
    inline void updateThread() override { m_run = true; }
    inline void stopThread() override { m_cleanup = true; }
};
}

using namespace imstk;

///
/// \brief TODO
///
class imstkModuleTest : public ::testing::Test
{
protected:
    LoopThreadObjectMock m_threadObject;
};

TEST_F(imstkModuleTest, GetName)
{
    EXPECT_EQ(m_threadObject.getName(), "modulemock");
}

TEST_F(imstkModuleTest, GetSetLoopDelay)
{
    m_threadObject.setLoopDelay(2);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 2);

    m_threadObject.setLoopDelay(0.003);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 0.003);

    m_threadObject.setLoopDelay(400000000);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 400000000);

    m_threadObject.setLoopDelay(0);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 0);

    m_threadObject.setLoopDelay(-5);
    EXPECT_GE(m_threadObject.getLoopDelay(), 0);
}

TEST_F(imstkModuleTest, SetFrequency)
{
    m_threadObject.setFrequency(60);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 1000.0 / 60.0);

    m_threadObject.setFrequency(0.003);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 1000.0 / 0.003);

    m_threadObject.setFrequency(400000000);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 1000.0 / 400000000);

    m_threadObject.setFrequency(0);
    EXPECT_EQ(m_threadObject.getLoopDelay(), 0);

    m_threadObject.setFrequency(-5);
    EXPECT_GE(m_threadObject.getLoopDelay(), 0);
}

TEST_F(imstkModuleTest, GetFrequency)
{
    m_threadObject.setLoopDelay(2);
    EXPECT_EQ(m_threadObject.getFrequency(), 1000.0 / 2);

    m_threadObject.setLoopDelay(0.003);
    EXPECT_EQ(m_threadObject.getFrequency(), 1000.0 / 0.003);

    m_threadObject.setLoopDelay(400000000);
    EXPECT_EQ(m_threadObject.getFrequency(), 1000.0 / 400000000);

    m_threadObject.setLoopDelay(0);
    EXPECT_EQ(m_threadObject.getFrequency(), 0);

    m_threadObject.setLoopDelay(-5);
    EXPECT_GE(m_threadObject.getFrequency(), 0);
}
TEST_F(imstkModuleTest, ControlModule)
{
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Inactive);

    m_threadObject.start();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Inactive);
    ASSERT_FALSE(m_threadObject.m_init);
    ASSERT_FALSE(m_threadObject.m_run);
    ASSERT_FALSE(m_threadObject.m_cleanup);

    m_threadObject.pause();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Inactive);
    ASSERT_FALSE(m_threadObject.m_init);
    ASSERT_FALSE(m_threadObject.m_run);
    ASSERT_FALSE(m_threadObject.m_cleanup);

    m_threadObject.stop();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Inactive);
    ASSERT_FALSE(m_threadObject.m_init);
    ASSERT_FALSE(m_threadObject.m_run);
    ASSERT_TRUE(m_threadObject.m_cleanup);

    auto t = std::thread([this] { m_threadObject.start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Running);
    ASSERT_TRUE(m_threadObject.m_init);
    ASSERT_TRUE(m_threadObject.m_run);

    m_threadObject.pause();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Paused);
    m_threadObject.m_run = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_TRUE(m_threadObject.m_init);
    ASSERT_FALSE(m_threadObject.m_run);

    m_threadObject.start();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Running);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_TRUE(m_threadObject.m_init);
    ASSERT_TRUE(m_threadObject.m_run);

    m_threadObject.stop();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Inactive);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_TRUE(m_threadObject.m_init);
    ASSERT_TRUE(m_threadObject.m_run);
    ASSERT_TRUE(m_threadObject.m_cleanup);

    t.join();
    m_threadObject.m_init = m_threadObject.m_run = m_threadObject.m_cleanup = false;

    t = std::thread([this] { m_threadObject.start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Running);
    ASSERT_TRUE(m_threadObject.m_init);
    ASSERT_TRUE(m_threadObject.m_run);
    ASSERT_FALSE(m_threadObject.m_cleanup);

    m_threadObject.pause();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Paused);
    m_threadObject.m_run = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_TRUE(m_threadObject.m_init);
    ASSERT_FALSE(m_threadObject.m_run);
    ASSERT_FALSE(m_threadObject.m_cleanup);

    m_threadObject.stop();
    ASSERT_EQ(m_threadObject.getStatus(), ThreadStatus::Inactive);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ASSERT_TRUE(m_threadObject.m_init);
    ASSERT_FALSE(m_threadObject.m_run);
    ASSERT_TRUE(m_threadObject.m_cleanup);

    t.join();
}

///
/// \brief TODO
///
int
imstkModuleTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
