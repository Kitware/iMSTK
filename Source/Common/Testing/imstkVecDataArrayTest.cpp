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

#define IMSTK_CHECK_ARRAY_RANGE
#include "imstkDataArray.h"
#include "imstkVecDataArray.h"
#undef IMSTK_CHECK_ARRAY_RANGE

using namespace imstk;

namespace
{
template<class T, int N>
bool
isEqualTo(const VecDataArray<T, N>& original, std::initializer_list<Eigen::Matrix<T, N, 1>>&& p)
{
    VecDataArray<T, N> other(p);
    if (original.size() != other.size())
    {
        return false;
    }
    for (int i = 0; i < original.size(); ++i)
    {
        if (original[i] != other[i])
        {
            return false;
        }
    }
    return true;
}
}

TEST(imstkVecDataArrayTest, Constructors)
{
    using SampleType = VecDataArray<int, 2>;
    EXPECT_NO_THROW(SampleType sample);

    // Note, size and capacity return the underlying total size e.g. for
    // VecDataArray<int, 2>,  2 * n
    VecDataArray<int, 2> a;
    EXPECT_EQ(0, a.size());
    EXPECT_EQ(2, a.getCapacity());
    EXPECT_EQ(2, a.getNumberOfComponents());

    VecDataArray<int, 2> b{ Vec2i(0, 1), Vec2i(2, 3) };
    EXPECT_EQ(2, b.size());
    EXPECT_EQ(4, b.getCapacity());
    EXPECT_TRUE(isEqualTo(b, { Vec2i(0, 1), Vec2i(2, 3) }));

    VecDataArray<int, 2> c(128);
    EXPECT_EQ(128, c.size());
    EXPECT_EQ(128 * 2, c.getCapacity());

    auto                 ptr = b.getPointer();
    VecDataArray<int, 2> d(std::move(b));
    EXPECT_EQ(2, d.size());
    EXPECT_EQ(4, d.getCapacity());
    EXPECT_TRUE(isEqualTo(d, { Vec2i(0, 1), Vec2i(2, 3) }));
    EXPECT_EQ(ptr, d.getPointer());
}

TEST(imstkVecDataArrayTest, Accessors)
{
    VecDataArray<int, 2> a{ Vec2i(1, 1), Vec2i(2, 2), Vec2i(3, 3), Vec2i(4, 4) };

    EXPECT_EQ(Vec2i(3, 3), a[2]);
    EXPECT_EQ(Vec2i(1, 1), a[0]);

    a[3] = Vec2i(6, 6);
    EXPECT_EQ(Vec2i(6, 6), a[3]);

    // Checked Arrays only
    EXPECT_ANY_THROW(a[4]);
}

TEST(imstkVecDataArrayTest, AccessorsConst)
{
    const VecDataArray<int, 2> a{ Vec2i(1, 1), Vec2i(2, 2), Vec2i(3, 3), Vec2i(4, 4) };

    EXPECT_EQ(Vec2i(3, 3), a[2]);
    EXPECT_EQ(Vec2i(1, 1), a[0]);

    // Checked Arrays only
    EXPECT_ANY_THROW(a[4]);
}

TEST(imstkVecDataArrayTest, Assignment)
{
    VecDataArray<int, 2> a;
    a = { Vec2i(1, 2), Vec2i(3, 4), Vec2i(5, 6) };
    EXPECT_EQ(3, a.size());
    EXPECT_EQ(3 * 2, a.getCapacity());
    EXPECT_TRUE(isEqualTo(a, { Vec2i(1, 2), Vec2i(3, 4), Vec2i(5, 6) }));

    VecDataArray<int, 2> b{ Vec2i(0, 2), Vec2i(4, 6) };

    b = a;
    EXPECT_TRUE(isEqualTo(b, { Vec2i(1, 2), Vec2i(3, 4), Vec2i(5, 6) }));
    EXPECT_EQ(a.size(), b.size());
    EXPECT_EQ(a.getCapacity(), b.getCapacity());
    EXPECT_NE(a.getPointer(), b.getPointer());
}

TEST(imstkVecDataArrayTest, Mapping)
{
    std::vector<Vec2i> other{ { -1, -2, }, { -3, -4 } };
    {
        VecDataArray<int, 2> a{ Vec2i(1, 2) };
        a.setData(other.data(), static_cast<int>(other.size()));
        EXPECT_EQ(2, a.size());
        EXPECT_EQ(2 * 2, a.getCapacity());
        EXPECT_TRUE(isEqualTo(a, { Vec2i(-1, -2), Vec2i(-3, -4) }));
        EXPECT_EQ(other.data(), a.getPointer());

        VecDataArray<int, 2> b = a;
        EXPECT_EQ(2, b.size());
        EXPECT_TRUE(isEqualTo(b, { Vec2i(-1, -2), Vec2i(-3, -4) }));
        EXPECT_EQ(other.data(), b.getPointer());
    }
}

TEST(imstkVecDataArrayTest, CapacityManagement)
{
    {
        VecDataArray<int, 2> a;
        EXPECT_EQ(0, a.size());
        EXPECT_EQ(2, a.getCapacity());
        a.push_back({ 0, 0 });
        EXPECT_EQ(1, a.size());
        EXPECT_EQ(2, a.getCapacity());
        for (int i = 1; i < 10; ++i)
        {
            a.push_back({ i, i });
            EXPECT_EQ(i + 1, a.size());
        }
        EXPECT_TRUE(isEqualTo(a, { { 0, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 }, { 6, 6 }, { 7, 7 }, { 8, 8 }, { 9, 9 } }));
    }
    {
        VecDataArray<int, 2> a;
        a.resize(100);
        EXPECT_EQ(100, a.size());
        EXPECT_EQ(100 * 2, a.getCapacity());
        a.resize(50);
        EXPECT_EQ(50, a.size());
        EXPECT_EQ(50 * 2, a.getCapacity());
        a.resize(0);
        EXPECT_EQ(0, a.size());
        EXPECT_EQ(1 * 2, a.getCapacity());
    }
    {
        VecDataArray<int, 2> a{ Vec2i(0, 2), Vec2i(4, 6) };
        a.reserve(256);
        EXPECT_EQ(2, a.size());
        EXPECT_EQ(256 * 2, a.getCapacity());
        a.reserve(100);
        EXPECT_EQ(2, a.size());
        EXPECT_EQ(256 * 2, a.getCapacity());
        a.squeeze();
        EXPECT_EQ(2, a.size());
        EXPECT_EQ(2 * 2, a.getCapacity());
    }
}

TEST(imstkVecDataArrayTest, Iterators)
{
    VecDataArray<int, 2> a;

    auto itBegin = a.begin();
    auto itEnd   = a.end();
    EXPECT_EQ(itBegin, itEnd);
    a = { Vec2i(1, 1), Vec2i(2, 2), Vec2i(3, 3) };
    auto it = a.begin();
    itEnd = a.end();
    int expected = 1;
    while (it != itEnd)
    {
        EXPECT_EQ(Vec2i(expected, expected), *it);
        ++it;
        ++expected;
    }

    // Checked Arrays only
    EXPECT_ANY_THROW(++it);
}

TEST(imstkVecDataArrayTest, Erase)
{
    VecDataArray<int, 2> a{ Vec2i(1, 1), Vec2i(2, 2), Vec2i(3, 3), Vec2i(4, 4) };
    a.erase(1);
    EXPECT_EQ(3, a.size());
    EXPECT_EQ(8, a.getCapacity());
    EXPECT_TRUE(isEqualTo(a, { Vec2i(1, 1), Vec2i(3, 3), Vec2i(4, 4) }));
    EXPECT_EQ(8, a.getCapacity());
    a.erase(-1);
    EXPECT_EQ(3, a.size());
    a.erase(20);
    EXPECT_EQ(3, a.size());
    a.erase(1);
    a.erase(0);
    EXPECT_TRUE(isEqualTo(a, { Vec2i(4, 4) }));
    EXPECT_EQ(8, a.getCapacity());
    a.erase(0);
    EXPECT_EQ(0, a.size());
    EXPECT_EQ(2, a.getCapacity());
}

TEST(imstkVecDataArrayTest, ExplicitCast)
{
    {
        VecDataArray<double, 2> a;
        VecDataArray<int, 2>    b;
        EXPECT_NO_THROW(b = a.cast<int>());
        EXPECT_EQ(a.size(), b.size());
    }

    {
        VecDataArray<int, 2> a{ Vec2i(1, 2), Vec2i(3, 4), Vec2i(5, 6), Vec2i(7, 8) };
        auto                 b = a.cast<double>();

        ASSERT_EQ(a.size(), b.size());
        EXPECT_EQ(IMSTK_DOUBLE, b.getScalarType());
        for (int i = 0; i < a.size(); ++i)
        {
            EXPECT_TRUE(a[i].cast<double>().isApprox(b[i]));
        }
    }
}

TEST(imstkVecDataArrayTest, ParameterCast)
{
    VecDataArray<int, 2> a{ Vec2i(1, 2), Vec2i(3, 4), Vec2i(5, 6), Vec2i(7, 8) };
    AbstractDataArray*   abstractA = &a;

    auto b = abstractA->cast(IMSTK_DOUBLE);

    EXPECT_EQ(IMSTK_DOUBLE, b->getScalarType());

    auto actualB = dynamic_cast<VecDataArray<double, 2>*>(b.get());
    ASSERT_NE(nullptr, actualB);
    ASSERT_EQ(a.size(), actualB->size());
    for (int i = 0; i < a.size(); ++i)
    {
        EXPECT_TRUE(a[i].cast<double>().isApprox((*actualB)[i]));
    }
}

TEST(imstkVecDataArrayTest, ResizeToOne)
{
    VecDataArray<int, 2> a;
    a.resize(1);
    EXPECT_EQ(1, a.size());
}