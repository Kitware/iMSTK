/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdObjectCollision.h"

using namespace imstk;

class NeedleEmbeddedCH;

namespace imstk
{
class Entity;
} // namespace imstk

///
/// \class NeedleInteraction
///
/// \brief Defines interaction between a NeedleObject and PbdObject
///
class NeedleInteraction : public PbdObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<Entity> tissueObj,
                      std::shared_ptr<Entity> needleObj,
                      std::shared_ptr<Entity> threadObj);
    ~NeedleInteraction() override = default;

    IMSTK_TYPE_NAME(NeedleInteraction)

    bool initialize() override;

    void stitch();

private:
    std::shared_ptr<Entity> m_threadObj;
};