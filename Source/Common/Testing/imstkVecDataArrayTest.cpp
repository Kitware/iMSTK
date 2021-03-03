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

#include "imstkDataArray.h"
#include "imstkVecDataArray.h"

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

    VecDataArray<int, 2> b{ imstk::Vec2i{ 0, 1 }, { 2, 3 } };
    EXPECT_EQ(2, b.size());
    EXPECT_EQ(4, b.getCapacity());
    EXPECT_TRUE(isEqualTo(b, { imstk::Vec2i{ 0, 1, }, { 2, 3 } }));

    VecDataArray<int, 2> c(128);
    EXPECT_EQ(128, c.size());
    EXPECT_EQ(128 * 2, c.getCapacity());

    auto                 ptr = b.getPointer();
    VecDataArray<int, 2> d(std::move(b));
    EXPECT_EQ(2, d.size());
    EXPECT_EQ(4, d.getCapacity());
    EXPECT_TRUE(isEqualTo(d, { imstk::Vec2i{ 0, 1, }, { 2, 3 } }));
    EXPECT_EQ(ptr, d.getPointer());
}

TEST(imstkVecDataArrayTest, Assignment)
{
    VecDataArray<int, 2> a;
    a = { imstk::Vec2i{ 1, 2, }, { 3, 4 }, { 5, 6 } };
    EXPECT_EQ(3, a.size());
    EXPECT_EQ(3 * 2, a.getCapacity());
    EXPECT_TRUE(isEqualTo(a, { imstk::Vec2i{ 1, 2, }, { 3, 4 }, { 5, 6 } }));

    VecDataArray<int, 2> b{ imstk::Vec2i{ 0, 2, }, { 4, 6 } };

    b = a;
    EXPECT_TRUE(isEqualTo(b, { imstk::Vec2i{ 1, 2, }, { 3, 4 }, { 5, 6 } }));
    EXPECT_EQ(a.size(), b.size());
    EXPECT_EQ(a.getCapacity(), b.getCapacity());
    EXPECT_NE(a.getPointer(), b.getPointer());
}

TEST(imstkVecDataArrayTest, Mapping)
{
    std::vector<imstk::Vec2i> other{ { -1, -2, }, { -3, -4 } };
    {
        VecDataArray<int, 2> a{ imstk::Vec2i{ 1, 2, } };
        a.setData(other.data(), static_cast<int>(other.size()));
        EXPECT_EQ(2, a.size());
        EXPECT_EQ(2 * 2, a.getCapacity());
        EXPECT_TRUE(isEqualTo(a, { imstk::Vec2i{ -1, -2, }, { -3, -4 } }));
        EXPECT_EQ(other.data(), a.getPointer());

        VecDataArray<int, 2> b = a;
        EXPECT_EQ(2, b.size());
        EXPECT_TRUE(isEqualTo(b, { imstk::Vec2i{ -1, -2, }, { -3, -4 } }));
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
        VecDataArray<int, 2> a{ imstk::Vec2i{ 0, 2, }, { 4, 6 } };
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
    for (const auto& val : a)
    {
        GTEST_FAIL() << "Should not enter here";
    }

    auto itBegin = a.begin();
    auto itEnd   = a.end();
    EXPECT_EQ(itBegin, itEnd);
    a = { imstk::Vec2i{ 1, 1 }, { 2, 2 }, { 3, 3 } };
    auto it = a.begin();
    itEnd = a.end();
    int expected = 1;
    while (it != itEnd)
    {
        EXPECT_EQ(imstk::Vec2i(expected, expected), *it);
        ++it;
        ++expected;
    }
}

TEST(imstkVecDataArrayTest, ConstructorCast)
{
    VecDataArray<long, 2> a{ Vec2i{ 1, 2 }, { 3, 4 } };
    EXPECT_EQ(Vec2i(1, 2), a[0].cast<int>());
}

TEST(imstkVecDataArrayTest, Erase)
{
    VecDataArray<int, 2> a{ imstk::Vec2i{ 1, 1 }, { 2, 2 }, { 3, 3 }, {4,4} };
    a.erase(1);
    EXPECT_EQ(3, a.size());
    EXPECT_EQ(8, a.getCapacity());
    EXPECT_TRUE(isEqualTo(a, { imstk::Vec2i{ 1, 1, }, { 3, 3 }, {4,4 } }));
    EXPECT_EQ(8, a.getCapacity());
    a.erase(-1);
    EXPECT_EQ(3, a.size());
    a.erase(20);
    EXPECT_EQ(3, a.size());
    a.erase(1);
    a.erase(0);
    EXPECT_TRUE(isEqualTo(a, { imstk::Vec2i{ 4, 4, } }));
    EXPECT_EQ(8, a.getCapacity());
    a.erase(0);
    EXPECT_EQ(0, a.size());
    EXPECT_EQ(2, a.getCapacity());
}

int
imstkVecDataArrayTest(int argc, char* argv[])
{
    // Init Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}