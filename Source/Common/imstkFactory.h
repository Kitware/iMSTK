/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <unordered_map>
#include <functional>
#include <string>
#include <memory>

namespace imstk
{
/// \brief Generic Factory class that can take objects with constructor parameters
///
/// All the objects in the factory need to be convertible to a common base class
///
/// \tparam T The base class type, is also the return type of create()
/// \tparam Args Parameter pack argument for the list of params used to pass into create
template<typename T, class ... Args>
class ObjectFactory
{
public:

    using BaseType = T;
    using Creator  = std::function<T(Args...)>; ///< Type of the function to generate a new object

    /// \brief tries to construct the object give name, it will forward the given paramters
    /// \param name Name to use for lookup
    /// \param args parameters to pass into the creation function
    /// \throws if name was not registered
    static T create(const std::string& name, Args&& ... args)
    {
        // at will throw if name doesn't exist in the map
        return registry().at(name)(std::forward<Args>(args)...);
    }

    /// \brief adds a new creation function to the factory
    /// \param name Name to use, will overwrite an already defined name
    /// \param c function to be called when create() is called with the given name
    static void add(const std::string& name, Creator c)
    {
        registry()[name] = std::move(c);
    }

    /// \return true if the name can be found in the registry
    static bool contains(const std::string& name)
    {
        return registry().find(name) != registry().cend();
    }

private:
    using Registry = std::unordered_map<std::string, Creator>;

    /// \brief static registry, let's us use static functions with a static data member
    static Registry& registry()
    {
        static Registry registry = {};
        return registry;
    }
};

/// \brief Templated class that can add to the object factory with objects that will
///        be generated via `std::make_shared`
///
/// As most objects in iMSTK are passed via std::shared_ptr this can be used as a default
/// way to create a construction function
///
/// \tparam T The base class type (see ObjectFactory)
/// \tparam U The class that should be generated here (needs to be a subclass of T)
/// \tparam Args constructor parameter types, these can then be pass in ObjectFactory::create
template<typename T, typename U, typename ... Args>
class SharedObjectRegistrar
{
public:
    /// \brief The constructor can automatically register the given class in the Factory
    /// For example it can be used in global scope in an implementation file
    /// \param name is the name that will be used in the factory for this class
    SharedObjectRegistrar(std::string name)
    {
        static_assert(std::is_base_of<T, U>::value,
            "U must be a subclass of T");
        ObjectFactory<std::shared_ptr<T>, Args ...>::add(name, [](Args&& ... args) { return std::make_shared<U>(std::forward<Args>(args)...); });
    }
};
} // namespace imstk
