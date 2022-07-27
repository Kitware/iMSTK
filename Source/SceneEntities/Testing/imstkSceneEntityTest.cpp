/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneEntity.h"
#include "imstkMacros.h"

#include <gtest/gtest.h>

using namespace imstk;

namespace
{
class MockEntity : public SceneEntity
{
public:
    IMSTK_TYPE_NAME(MockEntity)
};
} // namespace

TEST(SceneEntityTest, typenames) {
    MockEntity m;
    EXPECT_EQ(m.getTypeName(), MockEntity::getStaticTypeName());
}