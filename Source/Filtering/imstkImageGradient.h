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

///
/// \class ImageGradient
///
/// \brief This filter computes the gradient or magnitude using sobels kernel over an image
///
class ImageGradient : public GeometryAlgorithm
{
public:
    ImageGradient();
    virtual ~ImageGradient() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputImage(std::shared_ptr<ImageData> inputImage);

    ///
    /// \brief If on, a single channel magnitude image is output, if off, a 3 component gradient is produced
    ///
    imstkSetMacro(ComputeMagnitude, bool);
    imstkGetMacro(ComputeMagnitude, bool);

protected:
    void requestUpdate() override;

private:
    bool ComputeMagnitude;
};
}