/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdObjectCollision.h"

using namespace imstk;

class NeedleEmbedder;

namespace imstk
{
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
    NeedleInteraction(std::shared_ptr<Entity> tissueObj,
                      std::shared_ptr<Entity> needleObj,
                      const std::string&      collisionName = "");
    ~NeedleInteraction() override = default;

    IMSTK_TYPE_NAME(NeedleInteraction)

    std::shared_ptr<NeedleEmbedder> getEmbedder() const { return m_embedder; }

    void setFriction(const double friction);
    double getFriction() const;

    void setNeedleCompliance(const double compliance);
    double getNeedleCompliance() const;

    void setStaticFrictionForceThreshold(const double force);
    const double getStaticFrictionForceThreshold() const;

    void setPunctureForceThreshold(const double forceThreshold);
    const double getPunctureForceThreshold() const;

    bool initialize() override;

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<NeedleEmbedder> m_embedder;
    std::shared_ptr<TaskNode>       m_embedderNode = nullptr;
};