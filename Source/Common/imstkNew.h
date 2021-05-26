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
/// \brief std::shared_ptr<T> obj = std::make_shared<T>(); equivalent, convenience class
/// for STL shared allocations. Cannot be used in overloaded polymorphic calls
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
    explicit imstkNew(Args&& ... args) : object(std::make_shared<T>(std::forward<Args>(args) ...)) { }

    // Move, reference to old is removed
    imstkNew(imstkNew&& obj) : object(obj.object) { obj.object = nullptr; }

public:
    T* operator->() const { return object.get(); }
    T& operator*() const { return *object.get(); }

    ///
    /// \brief Returns the raw pointer
    ///
    T* getPtr() const { return object.get(); }

    ///
    /// \brief Returns const ref to STL smart pointer
    ///
    const std::shared_ptr<T>& get() const { return object; }

    ///
    /// \brief Implicit conversion
    ///
    operator std::shared_ptr<T>() const { return object; }

    ///
    /// \brief Implicit conversion
    ///
    operator std::weak_ptr<T>() const { return object; }

    ///
    /// \brief Hack for multiple implicit conversions, does not work with overloads though
    /// as it won't know what to cast too
    ///
    template<typename U>
    operator std::shared_ptr<U>() const
    {
        static_assert(std::is_base_of<U, T>::value, "Argument U type not compatible with imstkNew<T>'s T");
        return std::dynamic_pointer_cast<U>(object);
    }

    ///
    /// \brief Hack for multiple implicit conversions, does not work with overloads though
    /// as it won't know what to cast too
    ///
    template<typename U>
    operator std::weak_ptr<U>() const
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