/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkFactory.h"

namespace imstk
{
class CollisionDetectionAlgorithm;
class Geometry;

/// \brief Manages and generates the CollisionDetectionAlgorithms.
///
/// The factory is a singleton and can be accessed anywhere.
/// Given a the name of a geometry/geometry algorithm this will generate
/// the appropriate object that should be executed. In by default the class
/// name is the name that is used to look up the algorithm.
/// The generation Will fail if the name is not known to the factory
///
/// There are multiple ways to register an collision detection algorithm,
/// preferred should be ...
/// \code
/// IMSTK_REGISTER_COLLISION_DETECTION(collisionDetectionType)
/// \endcode
/// will register the delegate for the class-name of the algorithm,
/// this will satisfy the default mechanism
/// If want to register a custom delegate with more customization you can use
/// \code
/// CDObjectRegistrar<delegateType> registrar("LookupTypeName");
/// \endcode
///
class CDObjectFactory : public ObjectFactory<std::shared_ptr<CollisionDetectionAlgorithm>>
{
public:
///
/// \brief attempts to create a new CD algorithm
/// \param collisionTypeName name of the class to look up
    static std::shared_ptr<CollisionDetectionAlgorithm> makeCollisionDetection(const std::string collisionTypeName);
///
/// \brief Get the CD type from the types of objects colliding
    static std::string getCDType(
        const Geometry& obj1,
        const Geometry& obj2);
};

/// \brief Auto registration class
/// \tparam T type of the class to register
template<typename T>
using CDObjectRegistrar = SharedObjectRegistrar<CollisionDetectionAlgorithm, T>;

#define IMSTK_REGISTER_COLLISION_DETECTION(objType) CDObjectRegistrar<objType> _imstk_registercd ## objType(#objType)
} // namespace imstk
