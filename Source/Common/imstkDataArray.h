/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkAbstractDataArray.h"
#include "imstkMath.h"
#include "imstkMacros.h"

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

    using ScalarType = T;
    using ValueType  = T;
    static constexpr int NumComponents = 1;

    class iterator
    {
    public:
        using self_type  = iterator;
        using value_type = T;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using pointer   = T*;
        using reference = T&;

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
        using value_type = T;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using pointer   = T*;
        using reference = const T&;

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

        self_type operator++(int junk)
        {
            ptr_++;
#ifdef IMSTK_CHECK_ARRAY_RANGE
            if ((end_ - ptr_) < 0) { throw std::runtime_error("iterator past bounds"); }
#endif
            return *this;
        }

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
    DataArray(std::initializer_list<U> list) : AbstractDataArray(static_cast<int>(list.size())), m_mapped(false), m_data(new T[list.size()])
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

    ///
    /// \brief Copy Constructor
    /// \note The Copy constructor *does not* copy the event subscribers of this object
    ///
    DataArray(const DataArray& other) : AbstractDataArray(other)
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

    DataArray(DataArray&& other)
    {
        m_mapped       = other.m_mapped;
        m_size         = other.m_size;
        m_capacity     = other.m_capacity;
        m_scalarType   = other.m_scalarType;
        m_data         = other.m_data; // Take the others buffer
        other.m_mapped = true;         // The others destructor should then not delete
    }

    ~DataArray() override
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
        if (m_mapped)
        {
            return;
        }

        if (size <= m_capacity)
        {
            m_size = size;
        }
        else
        {
            const T* oldData = m_data;
            m_data = new T[size];
            std::copy_n(oldData, m_size, m_data);
            delete[] oldData;
            m_size = m_capacity = size;
        }
    }

    ///
    /// \brief Fill the array with the specified value
    ///
    inline void fill(const T& val) { std::fill_n(m_data, m_size, val); }

    ///
    /// \brief Resize to current size
    ///
    virtual inline void squeeze()
    {
        const T* oldData = m_data;
        m_data = new T[m_size];
        std::copy_n(oldData, m_size, m_data);
        delete[] oldData;
        m_capacity = m_size;
    }

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
        if (newSize > m_capacity)   // If the new size exceeds capacity
        {
            resize(m_capacity * 2); // Conservative/copies values
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
        if (newSize > m_capacity)    // If the new size exceeds capacity
        {
            reserve(m_capacity * 2); // Conservative/copies values
        }
        m_size = newSize;
        m_data[newSize - 1] = val;
    }

    ///
    /// \brief begin(), end() to mirror std::vector
    ///@{
    iterator begin() { return iterator(m_data, m_data + m_size); }

    const_iterator begin() const { return const_iterator(m_data, m_data + m_size); }

    iterator end() { return iterator(m_data + m_size, m_data + m_size); }

    const_iterator end() const { return const_iterator(m_data + m_size, m_data + m_size); }

    const_iterator cbegin() const { return const_iterator(m_data, m_data + m_size); }

    const_iterator cend() const { return const_iterator(m_data + m_size, m_data + m_size); }
    ///@}

    ///
    /// \brief Allocates extra capacity, for the number of values, conservative reallocate
    ///
    inline void reserve(const int capacity) override
    {
        if (m_mapped) { return; }
        if (capacity <= m_capacity) { return; }

        const int currSize = m_size;
        resize(capacity);  // Reallocate
        m_size = currSize; // Keep current size
    }

    inline T* getPointer() { return m_data; }
    inline void* getVoidPointer() override { return static_cast<void*>(m_data); }

    inline T& operator[](const size_t pos)
    {
#ifdef IMSTK_CHECK_ARRAY_RANGE
        if (pos >= static_cast<size_t>(m_size)) { throw std::out_of_range("Index out of range"); }
#endif
        return m_data[pos];
    }

    inline const T& operator[](const size_t pos) const
    {
#ifdef IMSTK_CHECK_ARRAY_RANGE
        if (pos >= static_cast<size_t>(m_size)) { throw std::out_of_range("Index out of range"); }
#endif
        return m_data[pos];
    }

    ///
    /// \return the item at the given position
    /// \note as opposed to the std::vector bounds checking is only done when IMSTK_CHECK_ARRAY_RANGE is set
    ///
    inline T& at(const size_t pos)
    {
#ifdef IMSTK_CHECK_ARRAY_RANGE
        if (pos >= static_cast<size_t>(m_size)) { throw std::out_of_range("Index out of range"); }
#endif
        return m_data[pos];
    }

    ///
    /// \return the item at the given position
    /// \note as opposed to the std::vector bounds checking is only done when IMSTK_CHECK_ARRAY_RANGE is set
    ///
    inline const T& at(const size_t pos) const
    {
#ifdef IMSTK_CHECK_ARRAY_RANGE
        if (pos >= static_cast<size_t>(m_size)) { throw std::out_of_range("Index out of range"); }
#endif
        return m_data[pos];
    }

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
        // Delegate to the appropriate functions while maintaining state
        // No need to copy type as it's static and this will only be used
        // for `=` of equivalent types
        if (other.m_mapped)
        {
            setData(other.m_data, other.size());
        }
        else
        {
            if (m_mapped)
            {
                m_data     = nullptr;
                m_capacity = 0;
                m_size     = 0;
                m_mapped   = false;
            }
            reserve(other.size());
            std::copy_n(other.m_data, other.m_size, m_data);
            m_size = other.m_size;
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

    inline virtual int getNumberOfComponents() const override { return NumComponents; }

    ///
    /// \brief Cast array to specific c++ type
    ///
    template<typename N>
    DataArray<N> cast()
    {
        if (m_mapped)
        {
            throw(std::runtime_error("Can't cast a mapped array"));
        }
        DataArray<N> result;
        result.reserve(size());
        for (auto& i : *this)
        {
            result.push_back(static_cast<N>(i));
        }
        return result;
    }

    ///
    /// \brief Cast array to the IMSTK type on the abstract interface
    ///
    std::shared_ptr<AbstractDataArray> cast(ScalarTypeId type) override
    {
        if (type == AbstractDataArray::m_scalarType)
        {
            return std::make_shared<DataArray<T>>(*this);
        }
        switch (type)
        {
            TemplateMacro(return std::make_shared<DataArray<IMSTK_TT>>(cast<IMSTK_TT>()));
        default:
            throw(std::runtime_error("Unknown scalar type"));
        }
    }

    ///
    /// \brief Polymorphic clone, shadows the declaration in the superclasss
    ///        but returns own type
    ///
    std::unique_ptr<DataArray<T>> clone()
    {
        return std::unique_ptr<DataArray<T>>(cloneImplementation());
    }

protected:

    bool m_mapped;
    T*   m_data;

private:

    DataArray<T>* cloneImplementation()
    {
        return new DataArray<T>(*this);
    };
};
} // namespace imstk
