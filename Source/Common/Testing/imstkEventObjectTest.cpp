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
    MockSender   m;
    MockReceiver r;

    connect(&m, MockSender::SignalOne, &r, &MockReceiver::receiverOne);
    connect(&m, MockSender::SignalTwo, &r, &MockReceiver::receiverTwo);

    m.postOne();

    EXPECT_THAT(r.items, ElementsAre(1));
    m.postTwo();
    EXPECT_THAT(r.items, ElementsAre(1, 2));

    disconnect(&m, &r, MockSender::SignalTwo);

    m.postOne();
    EXPECT_THAT(r.items, ElementsAre(1, 2, 1));

    m.postTwo();
    EXPECT_THAT(r.items, ElementsAre(1, 2, 1));
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
    MockSender   m;
    MockReceiver r;
    int          callCount = 0;

    imstk::connect<Event>(&m, MockSender::SignalOne, [&](imstk::Event*) { ++callCount; });

    EXPECT_EQ(0, callCount);
    m.postOne();

    EXPECT_EQ(1, callCount);
}

TEST(imstkEventObjectTest, PointerQueued)
{
    MockSender   m;
    MockReceiver r;

    queueConnect(&m, MockSender::SignalOne, &r, &MockReceiver::receiverOne);
    queueConnect(&m, MockSender::SignalTwo, &r, &MockReceiver::receiverTwo);

    m.postOne();
    m.postTwo();
    EXPECT_THAT(r.items, ElementsAre());

    r.doAllEvents();
    EXPECT_THAT(r.items, ElementsAre(1, 2));

    m.postOne();
    m.postTwo();
    EXPECT_THAT(r.items, ElementsAre(1, 2));

    r.doEvent();
    EXPECT_THAT(r.items, ElementsAre(1, 2, 1));

    r.doEvent();
    EXPECT_THAT(r.items, ElementsAre(1, 2, 1, 2));

    disconnect(&m, &r, MockSender::SignalTwo);

    m.postOne();
    m.postTwo();

    r.doAllEvents();
    EXPECT_THAT(r.items, ElementsAre(1, 2, 1, 2, 1));

    disconnect(&m, &r, MockSender::SignalOne);
    m.postOne();
    m.postTwo();
    r.doAllEvents();
    EXPECT_THAT(r.items, ElementsAre(1, 2, 1, 2, 1));
}

TEST(imstkEventObjectTest, PointerQueuedForeach)
{
    MockSender   m;
    MockReceiver r;

    queueConnect(&m, MockSender::SignalOne, &r, &MockReceiver::receiverOne);
    queueConnect(&m, MockSender::SignalTwo, &r, &MockReceiver::receiverTwo);

    m.postOne();
    m.postTwo();

    r.foreachEvent([&](Command c) { c.invoke(); });

    EXPECT_THAT(r.items, ElementsAre(1, 2));
}

TEST(imstkEventObjectTest, PointerQueuedForeachBackwards)
{
    MockSender   m;
    MockReceiver r;

    queueConnect(&m, MockSender::SignalOne, &r, &MockReceiver::receiverOne);
    queueConnect(&m, MockSender::SignalTwo, &r, &MockReceiver::receiverTwo);

    m.postOne();
    m.postTwo();

    r.rforeachEvent([&](Command c) { c.invoke(); });

    EXPECT_THAT(r.items, ElementsAre(2, 1));
}