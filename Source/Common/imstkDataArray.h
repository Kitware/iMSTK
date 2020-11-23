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
#include "imstkMath.h"
#include "imstkMacros.h"
//#include "imstkParallelReduce.h"

namespace imstk
{
///
/// \class DataArray
///
/// \brief Simple dynamic array implementation that also supports
/// event posting and viewing/facade
///
template<typename T>
class DataArray : public AbstractDataArray
{
public:
    class iterator
    {
    public:
        using self_type  = iterator;
        using value_type = T;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = int;

    public:
        iterator(value_type* ptr) : ptr_(ptr) { }

        self_type operator++()
        {
            self_type i = *this;
            ptr_++;
            return i;
        }

        self_type operator++(int junk)
        {
            ptr_++;
            return *this;
        }

        value_type& operator*() { return *ptr_; }

        value_type* operator->() { return ptr_; }

        bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }

        bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }

    private:
        value_type* ptr_;
    };

    class const_iterator
    {
    public:
        using self_type  = const_iterator;
        using value_type = T;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = int;

    public:
        const_iterator(value_type* ptr) : ptr_(ptr) { }

        self_type operator++()
        {
            self_type i = *this;
            ptr_++;
            return i;
        }

        self_type operator++(int junk) { ptr_++; return *this; }

        const value_type& operator*() { return *ptr_; }

        const value_type* operator->() { return ptr_; }

        bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }

        bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }

    private:
        value_type* ptr_;
    };

public:
    ///
    /// \brief Constructs an empty data array
    /// DataArray will never have capacity < 1
    ///
    DataArray() : m_mapped(false), m_data(new T[1])
    {
        setType(TypeTemplateMacro(T));
        m_capacity = 1;
    }

    ///
    /// \brief Constructs a data array
    ///
    DataArray(const int size) : AbstractDataArray(size), m_mapped(false), m_data(new T[size])
    {
        setType(TypeTemplateMacro(T));
    }

    ///
    /// \brief Constructs from intializer list
    ///
    template<typename U>
    DataArray(std::initializer_list<U> list) : AbstractDataArray(size), m_mapped(false), m_data(new T[list.size()])
    {
        int j = 0;
        for (auto i : list)
        {
            m_data[j] = i;
            j++;
        }
        setType(TypeTemplateMacro(T));
        m_size = m_capacity = static_cast<int>(list.size());
    }

    DataArray(const DataArray& other)
    {
        // Copy the buffer instead of the pointer
        m_mapped     = other.m_mapped;
        m_size       = other.m_size;
        m_capacity   = other.m_capacity;
        m_scalarType = other.m_scalarType;
        if (m_mapped)
        {
            m_data = other.m_data;
        }
        else
        {
            m_data = new T[m_size];
            std::copy_n(other.m_data, m_size, m_data);
        }
    }

    DataArray(const DataArray&& other)
    {
        m_mapped     = other.m_mapped;
        m_size       = other.m_size;
        m_capacity   = other.m_capacity;
        m_scalarType = other.m_scalarType;
        m_data       = other.m_data; // Take the others buffer
        // todo: Make sure destructor is not called on other
    }

    virtual ~DataArray() override
    {
        if (!m_mapped)
        {
            delete[] m_data;
            m_data = nullptr;
        }
    }

public:
    ///
    /// \brief Resize data array to hold exactly size number of values
    ///
    inline void resize(const int size) override
    {
        // Can't resize a mapped vector
        if (m_mapped || size == m_size)
        {
            return;
        }

        if (size == 0)
        {
            delete[] m_data;
            m_data     = new T[1];
            m_size     = 0;
            m_capacity = 1;
        }
        else
        {
            const T*  oldData = m_data;
            const int oldSize = m_size;

            m_data = new T[size];
            if (oldSize <= size) // If old vector is smaller than new, it will fit
            {
                std::copy_n(oldData, oldSize, m_data);
            }
            else // If old vector is larger than new, chop it off, only copy up to new size
            {
                std::copy_n(oldData, size, m_data);
            }
            delete[] oldData;
            m_size = m_capacity = size;
        }
    }

    ///
    /// \brief Resize to current size
    ///
    virtual inline void squeeze() { resize(m_size); }

    ///
    /// \brief Append the data array to hold the new value, resizes if neccesary
    ///
    inline void push_back(const T& val)
    {
        // Can't push back to a mapped vector
        if (m_mapped)
        {
            return;
        }

        const int newSize = m_size + 1;
        if (newSize > m_capacity) // If the new size exceeds capacity
        {
            m_capacity *= 2;
            resize(m_capacity); // Conservative/copies values
        }
        m_size = newSize;
        m_data[newSize - 1] = val;
    }

    inline void push_back(const T&& val) // Move
    {
        // Can't push back to a mapped vector
        if (m_mapped)
        {
            return;
        }

        const int newSize = m_size + 1;
        if (newSize > m_capacity) // If the new size exceeds capacity
        {
            m_capacity *= 2;
            resize(m_capacity); // Conservative/copies values
        }
        m_size = newSize;
        m_data[newSize - 1] = val;
    }

    iterator begin() { return iterator(m_data); }

    iterator end() { return iterator(m_data + m_size); }

    const_iterator cbegin() const { return const_iterator(m_data); }

    const_iterator cend() const { return const_iterator(m_data + m_size); }

    ///
    /// \brief Allocates extra capacity, for the number of values, conservative reallocate
    ///
    inline void reserve(const int size) override
    {
        if (m_mapped)
        {
            return;
        }

        const int currSize = m_size;
        resize(size);      // Reallocate
        m_size = currSize; // Keep current size
    }

    inline T* getPointer() { return m_data; }
    inline void* getVoidPointer() override { return static_cast<void*>(m_data); }

    inline T& operator[](const size_t pos) { return m_data[pos]; }
    inline const T& operator[](const size_t pos) const { return m_data[pos]; }

    ///
    /// \brief Allow initialization from initializer list, ie: DataArray<int> arr = { 1, 2 }
    ///
    template<typename U>
    DataArray<T>& operator=(std::initializer_list<U> list)
    {
        // If previously mapped, don't delete, just overwrite
        if (!m_mapped)
        {
            delete[] m_data;
        }
        m_data = new T[list.size()];
        int j = 0;
        for (auto i : list)
        {
            m_data[j] = i;
            j++;
        }
        m_size   = m_capacity = static_cast<int>(list.size());
        m_mapped = false;
        return *this;
    }

    DataArray& operator=(const DataArray& other)
    {
        m_mapped     = other.m_mapped;
        m_scalarType = other.m_scalarType;
        m_size       = other.m_size;
        m_capacity   = other.m_capacity;
        if (m_mapped)
        {
            m_data = other.m_data;
        }
        else
        {
            delete[] m_data;
            m_data = new T[m_size];
            std::copy_n(other.m_data, m_size, m_data);
        }
        return *this;
    }

    ///
    /// \brief Computes the range of a component of the vectors elements
    ///
    /*inline Vec2d getRange(const int component) const
    {
        ParallelUtils::RangeFunctor<DataArray<T, N>> pObj(vec);
        tbb::parallel_reduce(tbb::blocked_range<size_t>(0, vec.size()), pObj);
        return pObj.getRange();
        return Vec2d(0.0, 0.0);
    }*/

    ///
    /// \brief: Use the array as a facade to anothers memory
    /// \param: Pointer to external buffer
    /// \param: Number of values (per ValueType * number of components)
    /// \param: If manage is true, the data will be deleted when the array is, default false
    ///
    inline void setData(T* ptr, const int size)
    {
        if (!m_mapped)
        {
            delete[] m_data;
        }
        m_mapped = true;
        m_data   = ptr;
        m_size   = m_capacity = size;
    }

    inline virtual int getNumberOfComponents() const override { return 1; }

protected:
    bool m_mapped;
    T*   m_data;
};
}