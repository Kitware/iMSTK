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

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class CleanMesh
///
/// \brief This filter reduces a SurfaceMesh
///
class QuadricDecimate : public GeometryAlgorithm
{
public:
    QuadricDecimate();
    ~QuadricDecimate() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

    ///
    /// \brief Get/Set whether to preserve volume or not, default on
    ///@{
    imstkSetMacro(VolumePreserving, bool);
    imstkGetMacro(VolumePreserving, bool);
    ///@}

    ///
    /// \brief Set the target reduction ratio
    ///@{
    imstkSetMacro(TargetReduction, double);
    imstkGetMacro(TargetReduction, double);
///@}

protected:
    void requestUpdate() override;

private:
    bool   m_VolumePreserving;
    double m_TargetReduction;
};
} // namespace imstk