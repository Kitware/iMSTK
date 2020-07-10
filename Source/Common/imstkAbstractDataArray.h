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
#include "imstkSignal.h"

namespace imstk
{
///
/// \class AbstractDataArray
///
/// \brief This class serves as the base class of DataArray, for typeless use
///
class AbstractDataArray
{
// Users should not be able to construct
protected:
    AbstractDataArray() = default;

    ///
    /// \brief Ensure all observers are disconnected
    ///
    virtual ~AbstractDataArray() { disconnectAll(); };

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
    virtual size_t size() = 0;

    ///
    /// \brief Returns void pointer to data
    ///
    virtual void* getVoidPointer() = 0;

    ///
    /// \brief Returns the scalar type of this array
    ///
    inline const ScalarType getScalarType() { return m_scalarType; }

    ///
    /// \brief Converts the return value to a variant for base class access
    /// Would not recommend using this for performant code. Instead one should
    /// downcast to the type and use it there.
    ///
    virtual const Variant getVariantValue(size_t element) = 0;

public:
    ///
    /// \brief Setup an observer for the modified call, returns id (for later removal)
    ///
    inline int connect(std::function<void(void*, size_t)> func) { return modifiedSignal.connect(func); }

    ///
    /// \brief First emits nullptr, 0 to all observers then disconnects all
    ///
    inline void disconnectAll()
    {
        modifiedSignal.emit(nullptr, 0);
        modifiedSignal.disconnectAll();
    }

    ///
    /// \brief Disconnects a specific observer
    ///
    inline void disconnect(const int id) { modifiedSignal.disconnect(id); }

    ///
    /// \brief emits signal to all observers, informing them on the current address
    /// in memory and size of array
    ///
    inline void modified() { modifiedSignal.emit(getVoidPointer(), size()); }

protected:
    void setType(const ScalarType type) { this->m_scalarType = type; }

private:
    ScalarType m_scalarType;
    Signal<void*, size_t> modifiedSignal;
};
}