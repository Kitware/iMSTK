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

#include "imstkFactory.h"

using namespace imstk;

namespace
{
struct A
{
    virtual int val() { return 1; };
    virtual ~A() {};
};

struct B : public A
{
    virtual ~B() {};
    int val() override { return 2; };
};

struct C : public A
{
    virtual ~C() {};
    int val() override { return 3; };
};

struct AA
{
    AA(int i) : m_i(i) {}
    virtual int val() { return 1; };
    virtual int vali() { return m_i; };
    virtual ~AA() {};
    protected:
        int m_i;
};

struct BB : public AA
{
    BB(int i) : AA(i) {}
    virtual ~BB() {};
    int val() override { return 2; };
};

struct CC : public AA
{
    CC(int i) : AA(i) {}
    virtual ~CC() {};
    int val() override { return 3; };
};
} // namespace

TEST(FactoryTest, Instantiation)
{
    ObjectFactory<A>::add("a", [] () { return A(); });

    auto a = ObjectFactory<A>::create("a");
}

TEST(FactoryTest, DerivedClasses)
{
    using TestFactory = ObjectFactory<std::shared_ptr<A>>;
    TestFactory::add("b", []() { return std::make_shared<B>(); });
    TestFactory::add("c", []() { return std::make_shared<C>(); });

    auto b = TestFactory::create("b");
    ASSERT_NE(nullptr, b);
    EXPECT_EQ(2, b->val());

    auto c = TestFactory::create("c");
    EXPECT_EQ(3, c->val());
}

TEST(FactoryTest, RegistrarTest)
{
    using TestFactory = ObjectFactory<std::shared_ptr<A>>;
    {
        auto a = SharedObjectRegistrar<A, B>("b");
        auto b = SharedObjectRegistrar<A, C>("c");
    }

    auto b = TestFactory::create("b");
    ASSERT_NE(nullptr, b);
    EXPECT_EQ(2, b->val());

    auto c = TestFactory::create("c");
    EXPECT_EQ(3, c->val());
}

TEST(FactoryTest, RegistrarTestWParams)
{
    using TestFactory = ObjectFactory<std::shared_ptr<AA>, int>;
    {
        auto a = SharedObjectRegistrar<AA, BB, int>("b");
        auto b = SharedObjectRegistrar<AA, CC, int>("c");
    }

    auto b = TestFactory::create("b", 10);
    ASSERT_NE(nullptr, b);
    EXPECT_EQ(2, b->val());
    EXPECT_EQ(10, b->vali());

    auto c = TestFactory::create("c", 20);
    EXPECT_EQ(3, c->val());
    EXPECT_EQ(20, c->vali());
}
