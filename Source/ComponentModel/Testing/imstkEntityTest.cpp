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
    IMSTK_TYPE_NAME(TestComponent)
    TestComponent(const std::string& name = "TestComponent") : Component(name)
    {
    }
};
class TestComponent2 : public Component
{
public:
    IMSTK_TYPE_NAME(TestComponent)
    TestComponent2(const std::string& name = "TestComponent2") : Component(name)
    {
    }
};
} // namespace

TEST(EntityTest, NameTest)
{
    Entity ent1;
    EXPECT_EQ("Entity", ent1.getName());

    Entity ent2("TestEntity");
    EXPECT_EQ("TestEntity", ent2.getName());

    ent2.setName("TestEntity2");
    EXPECT_EQ("TestEntity2", ent2.getName());
}

TEST(EntityTest, AddComponentTest)
{
    auto ent = std::make_shared<Entity>();

    // Add component with return
    std::shared_ptr<TestComponent> comp = ent->addComponent<TestComponent>();
    EXPECT_EQ(comp, ent->getComponent(0));

    // Add component with name and return
    comp = ent->addComponent<TestComponent>("TestComponent2");
    EXPECT_EQ(comp, ent->getComponent(1));
    EXPECT_EQ(comp->getName(), "TestComponent2");

    // Add existing component
    comp = std::make_shared<TestComponent>();
    ent->addComponent(comp);
    EXPECT_EQ(comp, ent->getComponent(2));

    // Add null component
    EXPECT_DEATH(ent->addComponent(nullptr), "Tried to add nullptr component");
    // Add duplicate component
    EXPECT_DEATH(ent->addComponent(comp), "Tried to add component to entity twice");
}

TEST(EntityTest, GetComponentTest)
{
    auto                            ent   = std::make_shared<Entity>();
    std::shared_ptr<TestComponent>  comp0 = ent->addComponent<TestComponent>();
    std::shared_ptr<TestComponent2> comp1 = ent->addComponent<TestComponent2>();
    std::shared_ptr<TestComponent>  comp2 = ent->addComponent<TestComponent>();

    EXPECT_EQ(comp0, ent->getComponent(0));
    // Should return first of type
    EXPECT_EQ(comp0, ent->getComponent<TestComponent>());
    // Return n'th component of type
    EXPECT_EQ(comp2, ent->getComponentN<TestComponent>(1));

    // Get component that doesn't exist
    EXPECT_DEATH(ent->getComponent(20), "Component with index does not exist, index out of range");

    // Get all components of type
    std::vector<std::shared_ptr<TestComponent>> comps = ent->getComponents<TestComponent>();
    EXPECT_EQ(2, comps.size());
    EXPECT_EQ(comp0, comps[0]);
    EXPECT_EQ(comp2, comps[1]);

    // Get all components
    std::vector<std::shared_ptr<Component>> allComps = ent->getComponents();
    EXPECT_EQ(3, allComps.size());
    EXPECT_EQ(comp0, allComps[0]);
    EXPECT_EQ(comp1, allComps[1]);
    EXPECT_EQ(comp2, allComps[2]);
}

TEST(EntityTest, RemoveComponentTest)
{
    auto                           ent  = std::make_shared<Entity>();
    std::shared_ptr<TestComponent> comp = ent->addComponent<TestComponent>();
    EXPECT_EQ(comp, ent->getComponent(0));
    ent->removeComponent(comp);
    EXPECT_DEATH(ent->getComponent(0), "Component with index does not exist, index out of range");
}

TEST(EntityTest, ContainsComponentTest)
{
    auto                           ent   = std::make_shared<Entity>();
    std::shared_ptr<TestComponent> comp0 = ent->addComponent<TestComponent>();
    std::shared_ptr<TestComponent> comp1 = ent->addComponent<TestComponent>();
    EXPECT_EQ(true, ent->containsComponent(comp0));
    EXPECT_EQ(true, ent->containsComponent(comp1));

    EXPECT_EQ(true, ent->containsComponent<TestComponent>());

    ent->removeComponent(comp1);
    ent->removeComponent(comp0);

    EXPECT_EQ(false, ent->containsComponent(comp0));
    EXPECT_EQ(false, ent->containsComponent(comp1));
}