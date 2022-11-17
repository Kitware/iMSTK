/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "imstkEventObject.h"

using namespace imstk;
using testing::ElementsAre;

class MockSender : public imstk::EventObject
{
public:
    /* *INDENT-OFF* */
    SIGNAL(MockSender,SignalOne);

    SIGNAL(MockSender,SignalTwo);
    /* *INDENT-ON* */

    void postOne()
    {
        postEvent(imstk::Event(SignalOne()));
    }

    void postTwo()
    {
        postEvent(imstk::Event(SignalTwo()));
    }
};

class MockReceiver : public imstk::EventObject
{
public:
    void receiverOne(imstk::Event*)
    {
        items.push_back(1);
    }

    void receiverTwo(imstk::Event*)
    {
        items.push_back(2);
    }

    std::vector<int> items;
};

TEST(imstkEventObjectTest, PointerImmediate)
{
    auto m = std::make_shared<MockSender>();
    auto r = std::make_shared<MockReceiver>();

    connect(m, MockSender::SignalOne, r, &MockReceiver::receiverOne);
    connect(m, MockSender::SignalTwo, r, &MockReceiver::receiverTwo);

    m->postOne();

    EXPECT_THAT(r->items, ElementsAre(1));
    m->postTwo();
    EXPECT_THAT(r->items, ElementsAre(1, 2));

    disconnect(m, r, MockSender::SignalTwo);

    m->postOne();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1));

    m->postTwo();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1));
}

TEST(imstkEventObjectTest, SharedPointerImmediate)
{
    auto m = std::make_shared<MockSender>();
    auto r = std::make_shared<MockReceiver>();

    connect(m, MockSender::SignalOne, r, &MockReceiver::receiverOne);
    connect(m, MockSender::SignalTwo, r, &MockReceiver::receiverTwo);

    m->postOne();

    EXPECT_THAT(r->items, ElementsAre(1));
    m->postTwo();
    EXPECT_THAT(r->items, ElementsAre(1, 2));

    disconnect(m, r, MockSender::SignalTwo);

    m->postOne();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1));

    m->postTwo();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1));
}

TEST(imstkEventObjectTest, LambdaImmediate)
{
    auto m = std::make_shared<MockSender>();
    auto r = std::make_shared<MockReceiver>();
    int  callCount = 0;

    connect<Event>(m, MockSender::SignalOne, [&](Event*) { ++callCount; });

    EXPECT_EQ(0, callCount);
    m->postOne();

    EXPECT_EQ(1, callCount);
}

TEST(imstkEventObjectTest, PointerQueued)
{
    auto m = std::make_shared<MockSender>();
    auto r = std::make_shared<MockReceiver>();

    queueConnect(m, MockSender::SignalOne, r, &MockReceiver::receiverOne);
    queueConnect(m, MockSender::SignalTwo, r, &MockReceiver::receiverTwo);

    m->postOne();
    m->postTwo();
    EXPECT_THAT(r->items, ElementsAre());

    r->doAllEvents();
    EXPECT_THAT(r->items, ElementsAre(1, 2));

    m->postOne();
    m->postTwo();
    EXPECT_THAT(r->items, ElementsAre(1, 2));

    r->doEvent();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1));

    r->doEvent();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1, 2));

    disconnect(m, r, MockSender::SignalTwo);

    m->postOne();
    m->postTwo();

    r->doAllEvents();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1, 2, 1));

    disconnect(m, r, MockSender::SignalOne);
    m->postOne();
    m->postTwo();
    r->doAllEvents();
    EXPECT_THAT(r->items, ElementsAre(1, 2, 1, 2, 1));
}

TEST(imstkEventObjectTest, PointerQueuedForeach)
{
    auto m = std::make_shared<MockSender>();
    auto r = std::make_shared<MockReceiver>();

    queueConnect(m, MockSender::SignalOne, r, &MockReceiver::receiverOne);
    queueConnect(m, MockSender::SignalTwo, r, &MockReceiver::receiverTwo);

    m->postOne();
    m->postTwo();

    r->foreachEvent([&](Command c) { c.invoke(); });

    EXPECT_THAT(r->items, ElementsAre(1, 2));
}

TEST(imstkEventObjectTest, PointerQueuedForeachBackwards)
{
    auto m = std::make_shared<MockSender>();
    auto r = std::make_shared<MockReceiver>();

    queueConnect(m, MockSender::SignalOne, r, &MockReceiver::receiverOne);
    queueConnect(m, MockSender::SignalTwo, r, &MockReceiver::receiverTwo);

    m->postOne();
    m->postTwo();

    r->rforeachEvent([&](Command c) { c.invoke(); });

    EXPECT_THAT(r->items, ElementsAre(2, 1));
}

///
/// \brief Test that when a receiver is deconstructed it is removed
/// and others still work
///
TEST(imstkEventObjectTest, ReceiverDeconstruct)
{
    auto m  = std::make_shared<MockSender>();
    auto r0 = std::make_shared<MockReceiver>();
    auto r1 = std::make_shared<MockReceiver>();

    connect(m, MockSender::SignalOne, r0, &MockReceiver::receiverOne);
    connect(m, MockSender::SignalOne, r1, &MockReceiver::receiverTwo);

    // Invoke and handle
    m->postOne();
    r0->doAllEvents();
    r1->doAllEvents();

    EXPECT_EQ(1, r0->items.size());
    EXPECT_EQ(1, r1->items.size());

    r0 = nullptr; // r0 is gone

    // Invoke and handle only r1 now
    m->postOne();
    r1->doAllEvents();

    // r1 should increment to 2
    EXPECT_EQ(2, r1->items.size());
}