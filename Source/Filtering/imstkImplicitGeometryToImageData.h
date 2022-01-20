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
#include "imstkMath.h"

namespace imstk
{
class ImageData;
class ImplicitGeometry;

///
/// \class ImplicitGeometryToImageData
///
/// \brief This filter rasterizes an implicit function to image of
/// specified dimensions and bounds
///
class ImplicitGeometryToImageData : public GeometryAlgorithm
{
public:
    ImplicitGeometryToImageData();
    ~ImplicitGeometryToImageData() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    ///
    /// \brief Required input, port 0
    ///
    void setInputGeometry(std::shared_ptr<ImplicitGeometry> inputGeometry);

    imstkGetMacro(Dimensions, const Vec3i&);
    imstkGetMacro(Bounds, const Vec6d&);

    imstkSetMacro(Dimensions, const Vec3i&);
    imstkSetMacro(Bounds, const Vec6d&);

protected:
    void requestUpdate() override;

private:
    Vec3i m_Dimensions;
    Vec6d m_Bounds;
};
} // namespace imstk