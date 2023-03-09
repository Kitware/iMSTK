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
std::shared_ptr<Entity> makePbdEntity(const std::string& name, std::shared_ptr<Geometry> visualGeometry, std::shared_ptr<Geometry> collidingGeometry, std::shared_ptr<Geometry> physicsGeometry,
                                      std::shared_ptr<PbdSystem> system);

/**
 * @brief Construct a default style PBD based entity that uses the same geometry for all of
 * VisualModel, Collider, and PbdMethod.
 * @param geom
 * @param system
 * @return Entity constructed from provided geometry and PbdSystem
*/
std::shared_ptr<Entity> makePbdEntity(const std::string& name, std::shared_ptr<Geometry> geom, std::shared_ptr<PbdSystem> system);

/**
* @brief Construct a default RenderMaterial object for a tissue object.
* @param opacity Optionally provide an opacity value. Default value is 1.0.
*/
std::shared_ptr<RenderMaterial> makeTissueMaterial(double opacity = 1.0);
} // namespace SceneUtils
} // namespace imstk