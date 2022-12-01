/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRigidObjectCollision.h"

using namespace imstk;

///
/// \class NeedleInteraction
///
/// \brief Defines interaction between NeedleObject and PbdObject
///
class NeedleInteraction : public RigidObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<Entity>          tissueObj,
                      std::shared_ptr<RigidObject2>    needleObj,
                      const std::string&               collisionName = "");
    ~NeedleInteraction() override = default;
};