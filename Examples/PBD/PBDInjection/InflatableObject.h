/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdObject.h"
#include "imstkPbdConstraintContainer.h"

namespace imstk
{
class ImageData;
class SurfaceMesh;
class TetrahedralMesh;
class Texture;
}; // namespace imstk

using namespace imstk;

///
/// \class InflatableObject
///
/// \brief Inflatable object based on PBD, with inflatable volume and distance constraints
///
class InflatableObject : public PbdObject
{
public:
    enum class InflationType
    {
        Linear,
        Exponential,
        None
    };

    InflatableObject(const std::string& name, const Vec3d& tissueSize, const Vec3i& tissueDim, const Vec3d& tissueCenter);

public:
    IMSTK_TYPE_NAME(InflatableObject)

    bool initialize() override;

    ///
    /// \brief Perform injection on the tissue given tool tip position
    ///
    void inject(const Vec3d& toolTip, const double radius, double rate);

    ///
    /// \brief Switch between linear and exponential inflation type
    ///
    void switchInflationType();

    void setInflationRatio(double ratio) { m_inflationRatio = ratio; }
    void setInflationSize(double sigma) { m_sigma = sigma; }

    ///
    /// \brief set update affected constraints flag
    ///
    void setUpdateAffectedConstraint();

    void reset() override;

protected:
    ///
    /// \brief Spherically project the texture coordinates
    ///
    void setSphereTexCoords(const double uvScale);

    ///
    /// \brief find affected constraints id and distance in the injection area
    ///
    void findAffectedConstraint(const Vec3d& toolTip, const double radius);

    ///
    ///  \brief Compute weight Gaussian distribution
    ///
    inline double computeGaussianWeight(double x) { return 10 * exp(-0.5 * x * x / m_sigma / m_sigma) / m_sigma; }

protected:
    std::shared_ptr<TetrahedralMesh> m_objectTetMesh;
    std::shared_ptr<SurfaceMesh>     m_objectSurfMesh;

    std::shared_ptr<PbdConstraintContainer> m_constraintContainer;
    std::vector<std::pair<int, double>>     m_constraintIDandWeight;

    bool m_affectedAreaUpdated = false;

    double m_inflationRatio = 1.0;
    double m_sigma = 1.0;

    InflationType m_inflationType = InflationType::Linear;
};