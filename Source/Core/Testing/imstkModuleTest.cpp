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
#include "gmock/gmock.h"

#include "imstkMath.h"
#include "imstkModule.h"

#include <thread>
#include <chrono>

namespace imstk
{
class ModuleMock : public Module
{
public:

    ModuleMock() : Module("modulemock") {}
    ~ModuleMock() = default;

    bool m_init = false;
    bool m_run = false;
    bool m_cleanup = false;

protected:

    inline void initModule() final { m_init = true; }
    inline void runModule() final { m_run = true; }
    inline void cleanUpModule() final { m_cleanup = true; }
};
}

using namespace imstk;

class imstkModuleTest : public ::testing::Test
{
protected:
    ModuleMock m_module;
};

TEST_F(imstkModuleTest, GetName)
{
    EXPECT_EQ(m_module.getName(), "modulemock");
}

TEST_F(imstkModuleTest, GetSetLoopDelay)
{
    m_module.setLoopDelay(2);
    EXPECT_EQ(m_module.getLoopDelay(), 2);

    m_module.setLoopDelay(0.003);
    EXPECT_EQ(m_module.getLoopDelay(), 0.003);

    m_module.setLoopDelay(400000000);
    EXPECT_EQ(m_module.getLoopDelay(), 400000000);

    m_module.setLoopDelay(0);
    EXPECT_EQ(m_module.getLoopDelay(), 0);

    m_module.setLoopDelay(-5);
    EXPECT_GE(m_module.getLoopDelay(), 0);
}

TEST_F(imstkModuleTest, SetFrequency)
{
    m_module.setFrequency(60);
    EXPECT_EQ(m_module.getLoopDelay(), 1000.0/60.0);

    m_module.setFrequency(0.003);
    EXPECT_EQ(m_module.getLoopDelay(), 1000.0/0.003);

    m_module.setFrequency(400000000);
    EXPECT_EQ(m_module.getLoopDelay(), 1000.0/400000000);

    m_module.setFrequency(0);
    EXPECT_EQ(m_module.getLoopDelay(), 0);

    m_module.setFrequency(-5);
    EXPECT_GE(m_module.getLoopDelay(), 0);
}

TEST_F(imstkModuleTest, GetFrequency)
{
    m_module.setLoopDelay(2);
    EXPECT_EQ(m_module.getFrequency(), 1000.0/2);

    m_module.setLoopDelay(0.003);
    EXPECT_EQ(m_module.getFrequency(), 1000.0/0.003);

    m_module.setLoopDelay(400000000);
    EXPECT_EQ(m_module.getFrequency(), 1000.0/400000000);

    m_module.setLoopDelay(0);
    EXPECT_EQ(m_module.getFrequency(), 0);

    m_module.setLoopDelay(-5);
    EXPECT_GE(m_module.getFrequency(), 0);
}
TEST_F(imstkModuleTest, ControlModule)
{
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::INACTIVE);

    m_module.run();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::INACTIVE);
    ASSERT_FALSE(m_module.m_init);
    ASSERT_FALSE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    m_module.pause();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::INACTIVE);
    ASSERT_FALSE(m_module.m_init);
    ASSERT_FALSE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    m_module.end();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::INACTIVE);
    ASSERT_FALSE(m_module.m_init);
    ASSERT_FALSE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    auto t = std::thread([this] { m_module.start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::RUNNING);
    ASSERT_TRUE(m_module.m_init);
    ASSERT_TRUE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    m_module.pause();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::PAUSED);
    m_module.m_run = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_TRUE(m_module.m_init);
    ASSERT_FALSE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    m_module.run();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::RUNNING);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_TRUE(m_module.m_init);
    ASSERT_TRUE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    m_module.end();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::INACTIVE);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_TRUE(m_module.m_init);
    ASSERT_TRUE(m_module.m_run);
    ASSERT_TRUE(m_module.m_cleanup);

    t.join();
    m_module.m_init = m_module.m_run = m_module.m_cleanup = false;

    t = std::thread([this] { m_module.start(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::RUNNING);
    ASSERT_TRUE(m_module.m_init);
    ASSERT_TRUE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    m_module.pause();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::PAUSED);
    m_module.m_run = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_TRUE(m_module.m_init);
    ASSERT_FALSE(m_module.m_run);
    ASSERT_FALSE(m_module.m_cleanup);

    m_module.end();
    ASSERT_EQ(m_module.getStatus(), ModuleStatus::INACTIVE);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ASSERT_TRUE(m_module.m_init);
    ASSERT_FALSE(m_module.m_run);
    ASSERT_TRUE(m_module.m_cleanup);

    t.join();
}

int imstkModuleTest(int argc, char* argv[])
{
    // Init Google Test & Mock
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}
