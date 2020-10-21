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
/// \class ImageDistanceTransform
///
/// \brief This filter generates a signed or unsigned distance transform from a binary mask
///
class ImageDistanceTransform : public GeometryAlgorithm
{
public:
    ImageDistanceTransform();
    virtual ~ImageDistanceTransform() override = default;

public:
    std::shared_ptr<ImageData> getOutputImage() const;

    ///
    /// \brief Required input, port 0
    ///
    void setInputImage(std::shared_ptr<ImageData> refImage);

    imstkGetMacro(UseUnsigned, bool);
    imstkSetMacro(UseUnsigned, bool);

protected:
    void requestUpdate() override;

private:
    bool m_UseUnsigned = false;
};
}