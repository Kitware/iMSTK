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
class NeedleInteraction : public PbdObjectCollision
{
public:
    NeedleInteraction(std::shared_ptr<PbdObject>    tissueObj,
                      std::shared_ptr<NeedleObject> needleObj);
    ~NeedleInteraction() override = default;

    IMSTK_TYPE_NAME(NeedleInteraction)

    std::shared_ptr<TetraToLineMeshCD> getEmbeddingCD() const { return m_tetMeshCD; }
    std::shared_ptr<NeedleEmbeddedCH> getEmbeddingCH() const { return m_embeddedCH; }

    void setFriction(const double friction);
    double getFriction() const;

    void setCompliance(const double compliance);
    double getCompliance() const;

    void setStaticFrictionForceThreshold(const double force);
    const double getStaticFrictionForceThreshold() const;

    void setPunctureForceThreshold(const double forceThreshold);
    const double getPunctureForceThreshold() const;

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<TetraToLineMeshCD> m_tetMeshCD;
    std::shared_ptr<NeedleEmbeddedCH>  m_embeddedCH;

    std::shared_ptr<TaskNode> m_embeddingCDNode = nullptr;
    std::shared_ptr<TaskNode> m_embeddingCHNode = nullptr;
};