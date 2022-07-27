/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTypes.h"
#include "imstkEventObject.h"

namespace imstk
{
///
/// \class AbstractDataArray
///
/// \brief This class serves as the base class of DataArray, for typeless use
///
class AbstractDataArray : public EventObject
{
public:
    AbstractDataArray() : m_scalarType(IMSTK_VOID), m_size(0), m_capacity(0) { }

    AbstractDataArray(const int size) : m_scalarType(IMSTK_VOID), m_size(size), m_capacity(size) { }

    ///
    /// \brief Ensure all observers are disconnected
    ///
    virtual ~AbstractDataArray() { };

    // *INDENT-OFF*
    SIGNAL(AbstractDataArray, modified);
    // *INDENT-ON*

public:
    ///
    /// \brief Resizes the array, may reallocate
    ///
    virtual void resize(const int size) = 0;

    ///
    /// \brief Reserves a size for the array in memory, may reallocate
    ///
    virtual void reserve(const int size) = 0;

    ///
    /// \brief Returns void pointer to data
    ///
    virtual void* getVoidPointer() = 0;

    ///
    /// \brief Resizes to 0
    ///
    void clear() { resize(0); };

    ///
    /// \brief Get number of values/tuples
    ///
    inline int size() const { return m_size; }

    ///
    /// \brief Returns the scalar type of this array
    ///
    inline ScalarTypeId getScalarType() const { return m_scalarType; }

    ///
    /// \brief Return the capacity of the array
    ///
    inline int getCapacity() const { return m_capacity; }

    ///
    /// \brief Returns the number of components
    ///
    inline virtual int getNumberOfComponents() const { return 1; }

    ///
    /// \brief cast the content to the given imstk scalar type without
    ///        having to know the type of the enclosed array
    ///
    virtual std::shared_ptr<AbstractDataArray> cast(ScalarTypeId) = 0;

public:
    ///
    /// \brief emits signal to all observers, informing them on the current address
    /// in memory and size of array
    ///
    inline void postModified() { this->postEvent(Event(AbstractDataArray::modified())); }

protected:
    void setType(const ScalarTypeId type) { this->m_scalarType = type; }

protected:
    ScalarTypeId m_scalarType;
    int m_size;     // Number of values
    int m_capacity; // Capacity of the vector
};
} // namespace imstk