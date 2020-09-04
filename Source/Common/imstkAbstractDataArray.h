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

#include "imstkTypes.h"
#include "imstkEventObject.h"

namespace imstk
{
///
/// \class AbstractDataArray
///
/// \brief This class serves as the base class of DataArray, for typeless use
/// \todo: Component support, AOS/SOA design
///
class AbstractDataArray : public EventObject
{
// Users should not be able to construct
protected:
    AbstractDataArray() = default;

    ///
    /// \brief Ensure all observers are disconnected
    ///
    virtual ~AbstractDataArray() { };

public:
    ///
    /// \brief Resizes the array, may reallocate
    ///
    virtual void resize(const size_t count) = 0;

    ///
    /// \brief Reserves a size for the array in memory, may reallocate
    ///
    virtual void reserve(const size_t count) = 0;

    ///
    /// \brief Returns the number of elements in the array
    ///
    virtual size_t size() const = 0;

    ///
    /// \brief Returns void pointer to data
    ///
    virtual void* getVoidPointer() = 0;

    ///
    /// \brief Returns the scalar type of this array
    ///
    inline const ScalarType getScalarType() { return m_scalarType; }

public:
    ///
    /// \brief emits signal to all observers, informing them on the current address
    /// in memory and size of array
    ///
    inline void modified() { emit(Event(EventType::Modified)); }

protected:
    void setType(const ScalarType type) { this->m_scalarType = type; }

private:
    ScalarType m_scalarType;
};
}