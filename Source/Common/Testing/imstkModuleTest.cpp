/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkModule.h"

#include <g3log/g3log.hpp>
#include <gtest/gtest.h>

namespace imstk
{
///
/// \brief TODO
///
class ModuleObjectMock : public Module
{
public:
    ModuleObjectMock() { }
    ~ModuleObjectMock() override = default;

protected:
    bool initModule() override { return true; }
    void updateModule() override { }
    void uninitModule() override { }
};
} // namespace imstk

using namespace imstk;

///
/// \brief TODO
///
class imstkModuleTest : public ::testing::Test
{
protected:
    ModuleObjectMock m_moduleObject;
};

TEST_F(imstkModuleTest, GetDt)
{
    m_moduleObject.setDt(0.001);
    EXPECT_EQ(m_moduleObject.getDt(), 0.001);
}

TEST_F(imstkModuleTest, Pause)
{
    m_moduleObject.pause();
    EXPECT_EQ(m_moduleObject.getPaused(), true);
}

TEST_F(imstkModuleTest, Resume)
{
    m_moduleObject.pause();
    m_moduleObject.resume();
    EXPECT_EQ(m_moduleObject.getPaused(), false);
}

TEST_F(imstkModuleTest, GetExecutionType)
{
    m_moduleObject.setExecutionType(Module::ExecutionType::ADAPTIVE);
    EXPECT_EQ(m_moduleObject.getExecutionType(), Module::ExecutionType::ADAPTIVE);
}

TEST_F(imstkModuleTest, SetExecutionType)
{
    m_moduleObject.setExecutionType(Module::ExecutionType::PARALLEL);
    EXPECT_EQ(m_moduleObject.getExecutionType(), Module::ExecutionType::PARALLEL);
}

TEST_F(imstkModuleTest, Initialize)
{
    m_moduleObject.init();
    EXPECT_EQ(m_moduleObject.getInit(), true);
}

TEST_F(imstkModuleTest, UnInitialize)
{
    m_moduleObject.init();
    m_moduleObject.uninit();
    EXPECT_EQ(m_moduleObject.getInit(), false);
}
