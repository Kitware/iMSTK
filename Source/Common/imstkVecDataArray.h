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

#include "imstkDataArray.h"
#include "imstkMath.h"
#include "imstkLogger.h"
//#include "imstkParallelReduce.h"

namespace imstk
{
///
/// \class VecDataArray
///
/// \brief Simple dynamic array implementation that also supports
/// event posting and viewing/facade
///
template<typename T, int N>
class VecDataArray : public DataArray<T>
{
public:
    using VecType = Eigen::Matrix<T, N, 1>;

    class iterator
    {
    public:
        using self_type  = iterator;
        using value_type = VecType;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using pointer   = VecType*;
        using reference = VecType&;

    public:
        iterator(pointer ptr, pointer end) : ptr_(ptr), end_(end) { }

        self_type operator++()
        {
            self_type i = *this;
            ptr_++;
#ifdef IMSTK_CHECK_ARRAY_RANGE
            if ((end_ - ptr_) < 0) { throw std::runtime_error("iterator past bounds"); }
#endif
            return i;
        }

        self_type operator++(int junk)
        {
            ptr_++;
#ifdef IMSTK_CHECK_ARRAY_RANGE
            if ((end_ - ptr_) < 0) { throw std::runtime_error("iterator past bounds"); }
#endif
            return *this;
        }

        reference operator*() { return *ptr_; }

        pointer operator->() { return ptr_; }

        bool operator==(const self_type& rhs) const { return ptr_ == rhs.ptr_; }

        bool operator!=(const self_type& rhs) const { return ptr_ != rhs.ptr_; }

    private:
        pointer ptr_;
        pointer end_;
    };

    class const_iterator
    {
    public:
        using self_type  = const_iterator;
        using value_type = VecType;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using pointer   = VecType*;
        using reference = const VecType&;

    public:
        const_iterator(pointer ptr, pointer end) : ptr_(ptr), end_(end) { }

        self_type operator++()
        {
            self_type i = *this;
            ptr_++;
#ifdef IMSTK_CHECK_ARRAY_RANGE
            if ((end_ - ptr_) < 0) { throw std::runtime_error("iterator past bounds"); }
#endif
            return i;
        }

        self_type operator++(int junk) { ptr_++; return *this; }

        reference operator*() { return *ptr_; }

        const pointer operator->() { return ptr_; }

        bool operator==(const self_type& rhs) const { return ptr_ == rhs.ptr_; }

        bool operator!=(const self_type& rhs) const { return ptr_ != rhs.ptr_; }

    private:
        pointer ptr_;
        pointer end_;
    };

public:
    ///
    /// \brief Constructs an empty data array
    ///
    VecDataArray() : DataArray<T>(N), m_vecSize(0), m_vecCapacity(1), m_dataCast(reinterpret_cast<VecType*>(DataArray<T>::m_data)) { }

    ///
    /// \brief Constructs a data array of size
    ///
    VecDataArray(const int size) : DataArray<T>(size * N), m_vecSize(size), m_vecCapacity(size), m_dataCast(reinterpret_cast<VecType*>(DataArray<T>::m_data)) { }

    ///
    /// \brief Constructs from intializer list
    ///
    VecDataArray(std::initializer_list<Eigen::Matrix<T, N, 1>> list) : DataArray<T>(static_cast<int>(list.size() * N)),
        m_vecSize(static_cast<int>(list.size())),
        m_vecCapacity(static_cast<int>(list.size())),
        m_dataCast(reinterpret_cast<VecType*>(DataArray<T>::m_data))
    {
        int j = 0;
        for (auto i : list)
        {
            m_dataCast[j] = i;
            j++;
        }
    }

    VecDataArray(const VecDataArray& other)
    {
        DataArray<T>::m_mapped          = other.m_mapped;
        AbstractDataArray::m_size       = other.m_size;
        AbstractDataArray::m_capacity   = other.m_capacity;
        AbstractDataArray::m_scalarType = other.m_scalarType;
        if (DataArray<T>::m_mapped)
        {
            DataArray<T>::m_data = other.m_data;
        }
        else
        {
            DataArray<T>::m_data = new T[AbstractDataArray::m_capacity];
            std::copy_n(other.m_data, AbstractDataArray::m_size, DataArray<T>::m_data);
        }
        m_vecSize     = other.m_vecSize;
        m_vecCapacity = other.m_vecCapacity;
        m_dataCast    = reinterpret_cast<VecType*>(DataArray<T>::m_data);
    }

    VecDataArray(VecDataArray&& other)
    {
        DataArray<T>::m_mapped        = other.m_mapped;
        AbstractDataArray::m_size     = other.m_size;
        AbstractDataArray::m_capacity = other.m_capacity;
        m_vecSize     = other.m_vecSize;
        m_vecCapacity = other.m_vecCapacity;
        AbstractDataArray::m_scalarType = other.m_scalarType;
        DataArray<T>::m_data = other.m_data; // Take the others buffer
        m_dataCast     = other.m_dataCast;
        other.m_mapped = true;
    }

    virtual ~VecDataArray() override = default;

    ///
    /// \brief Templated copy the current array with a new internal data type, does not change the number
    ///        of components, pass in the new type as a template argument
    ///
    template<typename U>
    VecDataArray<U, N> cast()
    {
        if (DataArray<T>::m_mapped)
        {
            throw(std::runtime_error("Can't cast a mapped array"));
        }
        VecDataArray<U, N> other;
        other.reserve(size());
        for (auto item : *this)
        {
            other.push_back(item.template cast<U>());
        }
        return other;
    }

    std::shared_ptr<AbstractDataArray> cast(ScalarType type) override
    {
        if (type == AbstractDataArray::m_scalarType)
        {
            return std::make_shared<VecDataArray<T, N>>(*this);
        }
        switch (type)
        {
            TemplateMacro(return (std::make_shared<VecDataArray<IMSTK_TT, N>>(cast<IMSTK_TT>())));
        default:
            throw(std::runtime_error("Unknown scalar type"));
        }
    }

public:
    ///
    /// \brief Resize data array to hold exactly size number of values
    ///
    inline void resize(const int size) override
    {
        // Can't resize a mapped vector
        if (DataArray<T>::m_mapped)
        {
            return;
        }

        if (size == m_vecCapacity)
        {
            DataArray<T>::m_size = DataArray<T>::m_capacity;
            m_vecSize = m_vecCapacity;
            return;
        }

        if (size == 0)
        {
            DataArray<T>::resize(N);
            m_dataCast = reinterpret_cast<VecType*>(DataArray<T>::m_data);
            AbstractDataArray::m_size = m_vecSize = 0;
            m_vecCapacity = 1;
        }
        else
        {
            DataArray<T>::resize(size * N);
            m_dataCast = reinterpret_cast<VecType*>(DataArray<T>::m_data);
            m_vecSize  = m_vecCapacity = size;
        }
    }

    inline void fill(const VecType& val) { std::fill_n(m_dataCast, m_vecSize, val); }

    inline int size() const { return m_vecSize; }

    inline void squeeze() override { resize(m_vecSize); }

    ///
    /// \brief Append the data array to hold the new value, resizes if neccesary
    ///
    inline void push_back(const VecType& val)
    {
        // Can't push back to a mapped vector
        if (DataArray<T>::m_mapped)
        {
            return;
        }

        const int newVecSize = m_vecSize + 1;
        if (newVecSize > m_vecCapacity)              // If the new size exceeds capacity
        {
            VecDataArray::resize(m_vecCapacity * 2); // Conservative/copies values
        }
        m_vecSize = newVecSize;
        AbstractDataArray::m_size  = newVecSize * N;
        m_dataCast[newVecSize - 1] = val;
    }

    inline void push_back(const VecType&& val) // Move
    {
        // Can't push back to a mapped vector
        if (DataArray<T>::m_mapped)
        {
            return;
        }

        const int newVecSize = m_vecSize + 1;
        if (newVecSize > m_vecCapacity)              // If the new size exceeds capacity
        {
            VecDataArray::resize(m_vecCapacity * 2); // Conservative/copies values
        }
        m_vecSize = newVecSize;
        AbstractDataArray::m_size  = newVecSize * N;
        m_dataCast[newVecSize - 1] = val;
    }

    iterator begin() { return iterator(m_dataCast, m_dataCast + m_vecSize); }

    iterator end() { return iterator(m_dataCast + m_vecSize, m_dataCast + m_vecSize); }

    const_iterator cbegin() const { return const_iterator(m_dataCast, m_dataCast + m_vecSize); }

    const_iterator cend() const { return const_iterator(m_dataCast + m_vecSize, m_dataCast + m_vecSize); }

    ///
    /// \brief Allocates extra capacity, for the number of values, conservative reallocate
    ///
    inline void reserve(const int size) override
    {
        if (DataArray<T>::m_mapped)
        {
            return;
        }

        if (size < m_vecCapacity) { return; }

        const int currVecSize = m_vecSize;
        const int currSize    = AbstractDataArray::m_size;
        this->resize(size);      // Resize to desired capacity
        AbstractDataArray::m_size = currSize;
        m_vecSize = currVecSize; // Keep current size
    }

    inline VecType* getPointer() { return m_dataCast; }

    inline VecType& operator[](const size_t pos)
    {
#ifdef IMSTK_CHECK_ARRAY_RANGE
        if (pos >= m_vecSize) { throw std::runtime_error("Index out of range"); }
#endif
        return m_dataCast[pos];
    }

    inline const VecType& operator[](const size_t pos) const
    {
#ifdef IMSTK_CHECK_ARRAY_RANGE
        if (pos >= m_vecSize) { throw std::runtime_error("Index out of range"); }
#endif
        return m_dataCast[pos];
    }

    inline void erase(const int vecPos)
    {
        // If array is mapped or pos is not in bounds, don't erase
        if (DataArray<T>::m_mapped || vecPos >= m_vecSize || vecPos < 0)
        {
            return;
        }

        // If erasing the only element, clear
        const int newSize = AbstractDataArray::m_size - N;
        if (newSize == 0)
        {
            this->clear();
            return;
        }

        const int pos = vecPos * N; // Position in type T instead of VecType

        if (vecPos != m_vecSize - 1)
        {
            std::copy_n(DataArray<T>::m_data + pos + N, AbstractDataArray::m_size - pos - N, DataArray<T>::m_data + pos);
        }

        m_vecSize--;
        AbstractDataArray::m_size -= N;
    }

    ///
    /// \brief Allow initialization from initializer list, ie: DataArray<int> arr = { 1, 2 }
    ///
    template<typename U, int M>
    VecDataArray<T, N>& operator=(std::initializer_list<Eigen::Matrix<U, M, 1>> list)
    {
        // If previously mapped, don't delete, just overwrite
        if (!DataArray<T>::m_mapped)
        {
            delete[] DataArray<T>::m_data;
        }
        DataArray<T>::m_data = new T[list.size() * N];
        m_dataCast = reinterpret_cast<VecType*>(DataArray<T>::m_data);
        int j = 0;
        for (auto i : list)
        {
            m_dataCast[j] = i;
            j++;
        }
        m_vecSize = m_vecCapacity = static_cast<int>(list.size());
        AbstractDataArray::m_size = AbstractDataArray::m_capacity = m_vecSize * N;
        DataArray<T>::m_mapped    = false;
        return *this;
    }

    VecDataArray& operator=(const VecDataArray& other)
    {
        DataArray<T>::m_mapped = other.m_mapped;
        AbstractDataArray::m_scalarType = other.m_scalarType;
        AbstractDataArray::m_size       = other.m_size;
        AbstractDataArray::m_capacity   = other.m_capacity;
        m_vecSize     = other.m_vecSize;
        m_vecCapacity = other.m_vecCapacity;
        if (DataArray<T>::m_mapped)
        {
            DataArray<T>::m_data = other.m_data;
        }
        else
        {
            delete[] DataArray<T>::m_data;
            DataArray<T>::m_data = new T[AbstractDataArray::m_capacity];
            std::copy_n(other.m_data, AbstractDataArray::m_size, DataArray<T>::m_data);
        }
        m_dataCast = reinterpret_cast<VecType*>(DataArray<T>::m_data);
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
    inline void setData(VecType* ptr, const int size)
    {
        if (!DataArray<T>::m_mapped)
        {
            delete[] DataArray<T>::m_data;
        }

        DataArray<T>::m_mapped = true;
        DataArray<T>::m_data   = reinterpret_cast<T*>(ptr);
        m_dataCast = ptr;
        AbstractDataArray::m_size = AbstractDataArray::m_capacity = size * N;
        m_vecSize = m_vecCapacity = size;
    }

    inline int getNumberOfComponents() const override { return N; }

private:
    int      m_vecSize;
    int      m_vecCapacity;
    VecType* m_dataCast;
};
}
