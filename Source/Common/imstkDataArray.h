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

#pragma once

#include "imstkAbstractDataArray.h"
#include "imstkParallelReduce.h"

#include <vector>

namespace imstk
{
///
/// \class DataArray
///
/// \brief This class serves as a wrapping to STL vector, it also provides modified callback
/// \todo: Support tuples/components
///
template<class T>
class DataArray : public AbstractDataArray
{
public:
    using iterator       = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

public:
    ///
    /// \brief Constructs an empty data array
    ///
    DataArray() { setType(TypeTemplateMacro(T)); }

    ///
    /// \brief Constructs a data array with count elements
    ///
    DataArray(size_t count) : vec(count) { setType(TypeTemplateMacro(T)); }

    ///
    /// \brief Constructs from intializer list
    ///
    DataArray(std::initializer_list<T> list) : vec(list.begin(), list.end()) { setType(TypeTemplateMacro(T)); }

public:
    inline void resize(const size_t newCount) override { vec.resize(newCount); }

    inline void push_back(const T& val) { vec.push_back(val); }
    inline void push_back(const T&& val) { vec.push_back(val); }

    inline void erase(const const_iterator& element) { vec.erase(element); }

    inline void reserve(const size_t count) override { vec.reserve(count); }

    inline size_t size() const override { return vec.size(); }

    inline T* getPointer() { return vec.data(); }
    inline void* getVoidPointer() override { return static_cast<void*>(vec.data()); }

    inline const Variant getVariantValue(size_t i) const override { return Variant(vec[i]); }

    inline T& operator[](const size_t pos) { return vec[pos]; }
    inline const T& operator[](const size_t pos) const { return vec[pos]; }

    ///
    /// \brief Allow initialization from initializer list, ie: DataArray<int> arr = { 1, 2 }
    ///
    template<typename U>
    DataArray<U>& operator=(std::initializer_list<U> list)
    {
        vec = std::vector<U>(list.begin(), list.end());
        return *this;
    }

    ///
    /// \brief Computes the scalar range
    ///
    inline Vec2d getScalarRange() const
    {
        ParallelUtils::RangeFunctor<std::vector<T>> pObj(vec);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, vec.size()), pObj);
        return pObj.getRange();
    }

private:
    std::vector<T> vec;
};
}