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

///
/// \class ImageReslice
///
/// \brief Resamples an image using a transform
///
class ImageReslice : public GeometryAlgorithm
{
public:
    enum class InterpolateType
    {
        Linear,
        Cubic,
        NearestNeighbor
    };

public:
    ImageReslice();

    virtual ~ImageReslice() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    void setInputImage(std::shared_ptr<ImageData> inputData);

    imstkGetMacro(Transform, const Mat4d&);
    imstkGetMacro(InterpolationType, const InterpolateType&);
    imstkGetMacro(OutputBounds, const Vec6d&);
    imstkGetMacro(OutputSpacing, const Vec3d&);
    imstkGetMacro(OutputDimensions, const Vec3i&);

    ///
    /// \brief Set the transformation matrix
    ///
    imstkSetMacro(Transform, const Mat4d&);

    ///
    /// \brief Set the interpolation type to use when resampling
    ///
    imstkSetMacro(InterpolationType, const InterpolateType&);

    imstkSetMacro(OutputBounds, const Vec6d&);
    imstkSetMacro(OutputSpacing, const Vec3d&);
    imstkSetMacro(OutputDimensions, const Vec3i&);

protected:
    void requestUpdate() override;

private:
    Mat4d m_Transform;
    InterpolateType m_InterpolationType = InterpolateType::Linear;

    Vec6d m_OutputBounds     = Vec6d::Zero();
    Vec3d m_OutputSpacing    = Vec3d::Zero();
    Vec3i m_OutputDimensions = Vec3i::Zero();
};
}