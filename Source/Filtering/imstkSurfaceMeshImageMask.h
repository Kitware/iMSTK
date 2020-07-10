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
class SurfaceMesh;

///
/// \class ImagePolyDataMask
///
/// \brief This filter generates a binary mask from input surface mesh. Optionally
/// one may provide a reference image for which to use its spacing, origin, dimensions
/// It can also work with some geometry that is non-manifold, but results are ambiguous
///
class SurfaceMeshImageMask : public GeometryAlgorithm
{
public:
    SurfaceMeshImageMask();
    virtual ~SurfaceMeshImageMask() override = default;

public:
    ///
    /// \brief Optional input, used for information (dimensions, spacing, etc)
    ///
    void setReferenceImage(std::shared_ptr<ImageData> refImage);
    void setInputMesh(std::shared_ptr<SurfaceMesh> mesh);
    std::shared_ptr<ImageData> getOutputImage() const;

    imstkGetMacro(BorderExtent, int);
    imstkGetMacro(Dimensions, const Vec3i&);

    ///
    /// \brief Adds a border around the generated image of pixel amount
    /// border exists outside of bounds
    ///
    imstkSetMacro(BorderExtent, int);

    ///
    /// \brief The dimensions of the image to generate
    ///
    imstkSetMacro(Dimensions, const Vec3i&);

    ///
    /// \brief The dimensions of the image to generate
    ///
    void setDimensions(int dimX, int dimY, int dimZ) { setDimensions(Vec3i(dimX, dimY, dimZ)); }

protected:
    void requestUpdate() override;

private:
    Vec3i Dimensions    = Vec3i(-1, -1, -1);
    int   BorderExtent  = 1;
    bool  CheckManifold = true;
};
}