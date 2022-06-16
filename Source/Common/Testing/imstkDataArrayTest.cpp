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
#undef IMSTK_CHECK_ARRAY_RANGE

using namespace imstk;

template<class T>
bool
isEqualTo(const DataArray<T>& original, std::initializer_list<T>&& p)
{
    DataArray<T> other(p);
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

TEST(imstkDataArrayTest, Constructors)
{
    EXPECT_NO_THROW(DataArray<int> sample);

    DataArray<int> a;
    EXPECT_EQ(0, a.size());
    EXPECT_EQ(1, a.getCapacity());

    DataArray<int> b{ 0, 1, 2, 3 };
    EXPECT_EQ(4, b.size());
    EXPECT_EQ(4, b.getCapacity());
    EXPECT_TRUE(isEqualTo(b, { 0, 1, 2, 3 }));

    DataArray<int> c(128);
    EXPECT_EQ(128, c.size());
    EXPECT_EQ(128, c.getCapacity());

    DataArray<int> d(std::move(b));
    EXPECT_EQ(4, d.size());
    EXPECT_EQ(4, d.getCapacity());
    EXPECT_TRUE(isEqualTo(d, { 0, 1, 2, 3 }));

    EXPECT_EQ(1, DataArray<int>::NumComponents);
}

TEST(imstkDataArrayTest, Accessors)
{
    DataArray<int> b{ 0, 1, 2, 3 };

    EXPECT_EQ(2, b[2]);
    EXPECT_EQ(0, b[0]);

    b[3] = 4;
    EXPECT_EQ(4, b[3]);

    // Checked Arrays only
    EXPECT_ANY_THROW(b[4]);
}

TEST(imstkDataArrayTest, AccessorsConst)
{
    const DataArray<int> b{ 0, 1, 2, 3 };

    EXPECT_EQ(2, b[2]);
    EXPECT_EQ(0, b[0]);

    // Checked Arrays only
    EXPECT_ANY_THROW(b[4]);
}

TEST(imstkDataArrayTest, AssignmentSufficientSpace)
{
    DataArray<int> a;
    a = { 1, 2, 3, 4 };
    EXPECT_EQ(4, a.size());
    EXPECT_TRUE(isEqualTo(a, { 1, 2, 3, 4 }));

    DataArray<int>    b{ 0, 2, 4, 6, 8, 10 };
    DataArray<double> d{ 0, 2, 4, 6, 8, 10 };
    int               capacity = b.getCapacity();
    auto              ptr      = b.getPointer();

    b = a;
    EXPECT_TRUE(isEqualTo(b, { 1, 2, 3, 4 }));
    EXPECT_EQ(capacity, b.getCapacity());
    EXPECT_EQ(a.size(), b.size());
    EXPECT_EQ(ptr, b.getPointer());
}

TEST(imstkDataArrayTest, AssignmentIncreaseCapacity)
{
    DataArray<int> a{ 1, 2, 3, 4 };
    DataArray<int> b{ 0, 2, 4, 6, 8, 10 };

    a = b;
    EXPECT_TRUE(isEqualTo(a, { 0, 2, 4, 6, 8, 10 }));
    EXPECT_EQ(b.getCapacity(), a.getCapacity());
    EXPECT_EQ(a.size(), b.size());
}

TEST(imstkDataArrayTest, AssignmentMappedAndUnmapped)
{
    {
        SCOPED_TRACE("Assign mapped to unmapped");
        std::vector<int> other{ -1, -2, -3 };
        DataArray<int>   mapped{ 1, 2, 3, 4 };
        mapped.setData(other.data(), static_cast<int>(other.size()));
        DataArray<int> unmapped{ 0, 2, 4, 6, 8, 10 };
        unmapped = mapped;

        EXPECT_TRUE(isEqualTo(unmapped, { -1, -2, -3 }));
        EXPECT_EQ(unmapped.size(), mapped.size());
    }

    {
        SCOPED_TRACE("Assign unmapped to mapped");
        std::vector<int> other{ -1, -2, -3 };
        DataArray<int>   mapped{ 1, 2, 3, 4 };
        mapped.setData(other.data(), static_cast<int>(other.size()));
        DataArray<int> unmapped{ 0, 2, 4, 6, 8, 10 };
        mapped = unmapped;

        EXPECT_TRUE(isEqualTo(mapped, { 0, 2, 4, 6, 8, 10 }));
        EXPECT_EQ(unmapped.size(), mapped.size());
    }
}

TEST(imstkDataArrayTest, Mapping)
{
    std::vector<int> other{ -1, -2, -3 };
    {
        DataArray<int> a{ 1, 2, 3, 4 };
        a.setData(other.data(), static_cast<int>(other.size()));
        EXPECT_EQ(3, a.size());
        EXPECT_TRUE(isEqualTo(a, { -1, -2, -3 }));
        EXPECT_EQ(other.data(), a.getPointer());

        DataArray<int> b = a;
        EXPECT_EQ(3, b.size());
        EXPECT_TRUE(isEqualTo(b, { -1, -2, -3 }));
        EXPECT_EQ(other.data(), b.getPointer());
    }
}

TEST(imstkDataArrayTest, CapacityManagement)
{
    DataArray<int> a;
    EXPECT_EQ(0, a.size());
    EXPECT_EQ(1, a.getCapacity());
    a.push_back(0);
    EXPECT_EQ(1, a.size());
    EXPECT_EQ(1, a.getCapacity());
    for (int i = 1; i < 10; ++i)
    {
        a.push_back(i);
        EXPECT_EQ(i + 1, a.size());
    }
    EXPECT_TRUE(isEqualTo(a, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }));
}

TEST(imstkDataArrayTest, Resize)
{
    DataArray<int> a{ 0, 1, 2, 3 };
    a.resize(8);
    EXPECT_EQ(8, a.getCapacity());
    EXPECT_EQ(8, a.size());
    a.resize(16);
    EXPECT_EQ(16, a.size());
    EXPECT_EQ(16, a.getCapacity());
    a.resize(8);
    EXPECT_EQ(8, a.size());
    EXPECT_EQ(16, a.getCapacity());

    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(i, a[i]);
}
}

TEST(imstkDataArrayTest, Reserve)
{
    DataArray<int> a{ 1, 2, 3, 4 };
    a.reserve(256);
    EXPECT_EQ(4, a.size());
    EXPECT_EQ(256, a.getCapacity());
    a.reserve(100);
    EXPECT_EQ(256, a.getCapacity());
    EXPECT_EQ(4, a.size());
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(i + 1, a[i]);
}

    a.squeeze();
    EXPECT_EQ(4, a.getCapacity());
    EXPECT_EQ(4, a.size());
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(i + 1, a[i]);
}
}

TEST(imstkDataArrayTest, Iterators)
{
    DataArray<int> a;

    auto itBegin = a.begin();
    auto itEnd   = a.end();
    EXPECT_EQ(itBegin, itEnd);
    a = { 1, 2, 3, 4 };
    auto it = a.begin();
    itEnd = a.end();
    int expected = 1;
    while (it != itEnd)
    {
        EXPECT_EQ(expected, *it);
        *it = 0;
        EXPECT_EQ(a[expected - 1], 0);
        ++it;
        ++expected;
    }

    // Checked iterators only
    EXPECT_ANY_THROW(++it);
}

TEST(imstkDataArrayTest, ScalarTypeId)
{
    DataArray<int> a;
    EXPECT_EQ(IMSTK_INT, a.getScalarType());
    DataArray<double> b;
    EXPECT_EQ(IMSTK_DOUBLE, b.getScalarType());
}

TEST(imstkDataArrayTest, TypeCast)
{
    DataArray<int> a{ 1, 2, 3, 4 };

    auto b = a.cast<double>();

    ASSERT_EQ(IMSTK_DOUBLE, b.getScalarType());
    ASSERT_EQ(a.size(), b.size());

    for (int i = 0; i < a.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(static_cast<double>(a[i]), b[i]);
    }
}

TEST(imstkDataArrayTest, ParameterCast)
{
    DataArray<int>     a{ 1, 2, 3, 4 };
    AbstractDataArray* abstractA = &a;

    auto b = abstractA->cast(IMSTK_DOUBLE);
    ASSERT_NE(nullptr, b);

    auto actualB = dynamic_cast<DataArray<double>*>(b.get());
    ASSERT_NE(nullptr, actualB);
    ASSERT_EQ(IMSTK_DOUBLE, actualB->getScalarType());
    ASSERT_EQ(a.size(), actualB->size());

    for (int i = 0; i < a.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(static_cast<double>(a[i]), (*actualB)[i]);
    }
}

TEST(imstkDataArrayTest, ResizeToOne)
{
    DataArray<int> a;
    a.resize(1);
    EXPECT_EQ(1, a.size());
}

TEST(imstkDataArrayTest, RangedBasedFor)
{
    {
        DataArray<int> a{ 1, 2, 3, 4 };
        SCOPED_TRACE("NonConst Read");
        int expected = 1;
        for (const auto value : a)
        {
            EXPECT_EQ(value, expected++);
        }
    }

    {
        DataArray<int> a{ 1, 2, 3, 4 };
        SCOPED_TRACE("NonConst Write");
        int index = 0;
        for (auto& value : a)
        {
            value = 0;
            EXPECT_EQ(0, a[index]);
            ++index;
        }
    }

    {
        SCOPED_TRACE("Const Read");
        const DataArray<int> aConst{ 1, 2, 3, 4 };
        int                  expected = 1;
        for (const auto value : aConst)
        {
            EXPECT_EQ(value, expected++);
        }
    }
}