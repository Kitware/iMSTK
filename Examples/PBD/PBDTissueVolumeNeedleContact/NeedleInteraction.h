/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdRigidObjectCollision.h"

using namespace imstk;

class NeedleEmbeddedCH;
class NeedleObject;

namespace imstk
{
class PbdObject;
class TetraToLineMeshCD;
} // namespace imstk

///
/// \class NeedleInteraction
///
/// \brief Defines interaction between NeedleObject and PbdObject
///
class NeedleInteraction : public PbdRigidObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<PbdObject>    tissueObj,
                      std::shared_ptr<NeedleObject> needleObj);
    ~NeedleInteraction() override = default;

    IMSTK_TYPE_NAME(NeedleInteraction)

    std::shared_ptr<TetraToLineMeshCD> getEmbeddingCD() const { return tetMeshCD; }
    std::shared_ptr<NeedleEmbeddedCH> getEmbeddingCH() const { return embeddedCH; }

    ///
    /// \brief Setup connectivity of task graph
    ///
    virtual void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<TetraToLineMeshCD> tetMeshCD;
    std::shared_ptr<NeedleEmbeddedCH>  embeddedCH;

    std::shared_ptr<TaskNode> embeddingCDNode = nullptr;
    std::shared_ptr<TaskNode> embeddingCHNode = nullptr;
};