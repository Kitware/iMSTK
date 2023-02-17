/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollider.h"
#include "imstkEntity.h"
#include "imstkGeometry.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystem.h"
#include "imstkVisualModel.h"

#include <memory>

// using std::shared_ptr<T>;

namespace imstk
{
using EntityPtr    = std::shared_ptr<Entity>;
using GeometryPtr  = std::shared_ptr<Geometry>;
using PbdSystemPtr = std::shared_ptr<PbdSystem>;

namespace SceneUtils
{
/**
 * @brief Prepare a default style PBD based entity. It creates an Entity and adds
 * as components: VisualModel, Collider, PbdMethod.
 * @param visualGeometry
 * @param collidingGeometry
 * @param physicsGeometry
 * @param system
 * @return Entity constructed from provided geometries and PbdSystem
*/
EntityPtr makePbdEntity(const std::string& name, GeometryPtr visualGeometry, GeometryPtr collidingGeometry, GeometryPtr physicsGeometry, PbdSystemPtr system);

/**
 * @brief Construct a default style PBD based entity that uses the same geometry for all of
 * VisualModel, Collider, and PbdMethod.
 * @param geom
 * @param system
 * @return Entity constructed from provided geometry and PbdSystem
*/
EntityPtr makePbdEntity(const std::string& name, GeometryPtr geom, PbdSystemPtr system);
} // namespace SceneUtils
} // namespace imstk