/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"
#include "imstkEntity.h"

#include <gtest/gtest.h>

using namespace imstk;

namespace
{
class TestComponent : public Component
{
public:
    TestComponent(const std::string& name = "TestComponent") : Component(name)
    {
    }

protected:
    void init() override
    {
        isInitd = true;
    }

public:
    bool isInitd = false;
};

class TestBehaviour : public Behaviour<double>
{
public:
    TestBehaviour(const std::string& name = "TestBehaviour") : Behaviour<double>(name) { }

    void update(const double&) { updated = true; }
    void visualUpdate(const double&) { visualUpdated = true; }

    bool updated       = false;
    bool visualUpdated = false;
};
} // namespace

TEST(ComponentTest, NameTest)
{
    TestComponent comp0;
    EXPECT_EQ("TestComponent", comp0.getName());

    TestComponent comp1("TestComponent1");
    EXPECT_EQ("TestComponent1", comp1.getName());

    comp1.setName("TestComponent2");
    EXPECT_EQ("TestComponent2", comp1.getName());
}

TEST(ComponentTest, GetEntityTest)
{
    auto ent   = std::make_shared<Entity>();
    auto comp0 = ent->addComponent<TestComponent>();
    EXPECT_EQ(ent, comp0->getEntity().lock());
}

TEST(ComponentTest, InitializeTest)
{
    auto ent   = std::make_shared<Entity>();
    auto comp0 = ent->addComponent<TestComponent>();
    comp0->initialize();
    EXPECT_EQ(true, comp0->isInitd);
}

TEST(BehaviourTest, TestUpdate)
{
    TestBehaviour behaviour;
    behaviour.update(0.0);
    EXPECT_EQ(true, behaviour.updated);

    behaviour.visualUpdate(0.0);
    EXPECT_EQ(true, behaviour.visualUpdated);
}

TEST(LambdaBehaviourTest, TestLambdaUpdate)
{
    LambdaBehaviour behaviour;
    bool            isUpdated = false;
    behaviour.setUpdate([&](const double&)
        {
            isUpdated = true;
        });
    behaviour.update(0.0);
    EXPECT_EQ(true, isUpdated);

    bool isVisualUpdated = false;
    behaviour.setVisualUpdate([&](const double&)
        {
            isVisualUpdated = true;
        });
    behaviour.visualUpdate(0.0);
    EXPECT_EQ(true, isVisualUpdated);
}