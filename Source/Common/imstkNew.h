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

#include <memory>

namespace imstk
{
namespace expiremental
{
//template<typename T>
//using imstkSmartPtr = std::shared_ptr<T>;
//
//template<typename T>
//using imstkWeakPtr = std::weak_ptr<T>;
//
//template<typename T>
//using imstkUniquePtr = std::unique_ptr<T>;

///
/// \class imstkNew
///
/// \brief Convenience class for STL shared allocations.
///
template<class T>
class imstkNew
{
public:
    template<typename U>
    friend class imstkNew;

public:
    imstkNew() : object(std::make_shared<T>()) { }

    template<typename ... Args>
    explicit imstkNew(Args&&... args) : object(std::make_shared<T>(std::forward<Args>(args)...)) { }

    // Move, reference to old is removed
    imstkNew(imstkNew&& obj) : object(obj.object) { obj.object = nullptr; }

public:
    T* operator->() const { return object.get(); }
    T& operator*() const { return *static_cast<T*>(object); }

    ///
    /// \brief Returns the raw pointer
    ///
    T* getPtr() const { return object.get(); }

    ///
    /// \brief Returns const ref to STL smart pointer
    ///
    const std::shared_ptr<T>& get() const { return object; }

    ///
    /// \brief Implicit conversion, cannot do two implicit conversions in a row.
    /// Thus it's not possible to go from imstkNew<T> -> std::shared_ptr<U> with
    /// this function. Where T : U.
    ///
    operator std::shared_ptr<T>() const { return object; }

    ///
    /// \brief Two implicit conversions cannot be done in row.
    /// This provides one implicit conversion to base class type.
    /// It will not work with overloads though as it will not know which type to
    /// cast too.
    ///
    template<typename U>
    operator std::shared_ptr<U>() const
    {
        static_assert(std::is_base_of<U, T>::value, "Argument U type not compatible with imstkNew<T>'s T");
        return std::dynamic_pointer_cast<U>(object);
    }

private:
    imstkNew(const imstkNew<T>&)       = delete;
    void operator=(const imstkNew<T>&) = delete;
    std::shared_ptr<T> object;
};
}
}