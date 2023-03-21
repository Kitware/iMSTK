/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "gtest/gtest.h"

#include "imstkAccumulationBuffer.h"

TEST(imstkAccumulationBufferTest, Constructor)
{
    AccumulationBuffer<double> a;
    EXPECT_EQ(a.getCapacity(), 1024);
    EXPECT_NEAR(a.getAverage(), 0.0, 1e-6);
    AccumulationBuffer<int> b(256);
    EXPECT_EQ(b.getCapacity(), 256);
}

TEST(imstkAccumulationBufferTest, Average)
{
    AccumulationBuffer<double> a(4);
    a.pushBack(4);
    // The value not quite correct until the buffer is full
    EXPECT_NEAR(a.getAverage(), 1.0, 1e-6);
    a.pushBack(4);
    a.pushBack(4);
    a.pushBack(4);
    EXPECT_NEAR(a.getAverage(), 4.0, 1e-6);
    a.pushBack(1);
    a.pushBack(2);
    a.pushBack(3);
    EXPECT_NEAR(a.getAverage(), 2.5, 1e-6);
    a.pushBack(4);
    EXPECT_NEAR(a.getAverage(), 2.5, 1e-6);
}
