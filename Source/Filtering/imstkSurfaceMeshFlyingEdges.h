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
class ImageData;
class SurfaceMesh;

///
/// \class SurfaceMeshFlyingEdges
///
/// \brief This filter extracts a single isocontour from an imstkImageData
///
class SurfaceMeshFlyingEdges : public GeometryAlgorithm
{
public:
    SurfaceMeshFlyingEdges();
    virtual ~SurfaceMeshFlyingEdges() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputImage(std::shared_ptr<ImageData> inputImage);

    std::shared_ptr<SurfaceMesh> getOutputMesh() const;

    imstkGetMacro(IsoValue, double);

    ///
    /// \brief Get the value at which the surface should be produced
    ///
    imstkSetMacro(IsoValue, double);

protected:
    void requestUpdate() override;

private:
    double m_IsoValue = 0.0;
};
}