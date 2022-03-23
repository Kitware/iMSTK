/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

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
    /// \brief Creates a tetraheral grid
    ///
    void makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center);

    ///
    /// \brief Compute the texture coordinates of the tissue on a plane
    ///
    void setXZPlaneTexCoords(const double uvScale);

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