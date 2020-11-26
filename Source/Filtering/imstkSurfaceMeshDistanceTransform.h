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
/// \class SurfaceMeshDistanceTransform
///
/// \brief This filter computes brute force signed distance field by computing the nearest
/// point in one dataset to the other
///
class SurfaceMeshDistanceTransform : public GeometryAlgorithm
{
public:
    SurfaceMeshDistanceTransform();
    virtual ~SurfaceMeshDistanceTransform() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputMesh(std::shared_ptr<SurfaceMesh> surfMesh);

    std::shared_ptr<ImageData> getOutputImage();

    imstkGetMacro(Dimensions, const Vec3i&);
    imstkGetMacro(Bounds, const Vec6d&);
    imstkGetMacro(UseBounds, bool);
    imstkGetMacro(NarrowBanded, bool);
    imstkGetMacro(DilateSize, int);

    ///
    /// \brief Dimensions of distance transform to fill
    ///
    imstkSetMacro(Dimensions, const Vec3i&);

    ///
    /// \brief Optionally one may specify bounds, if not specified
    /// bounds of the input SurfaceMesh is used
    ///
    imstkSetMacro(Bounds, const Vec6d&);

    imstkSetMacro(UseBounds, bool);

    ///
    /// \brief If on, will compute only a narrow banded transform
    ///
    imstkSetMacro(NarrowBanded, bool);

    ///
    /// \brief Width of the band
    ///
    imstkSetMacro(DilateSize, int);

    void setDimensions(int dimX, int dimY, int dimZ) { setDimensions(Vec3i(dimX, dimY, dimZ)); }

protected:
    void requestUpdate() override;

private:
    Vec3i m_Dimensions = Vec3i(0, 0, 0);
    Vec6d m_Bounds;
    bool  m_UseBounds = false;

    int  m_DilateSize   = 4; ///> Only for narrow banded
    bool m_NarrowBanded = false;
};
}