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
/// \brief This filter computes exact signed distance fields using octrees
/// and psuedonormal computations. One might need to adjust the tolerance
/// depending on dataset scale.
///
/// Note that when setting the bounding box, a margin should be applied to the
/// bounds otherwise vertices or triangles on the bounds may be ignored
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

    ///
    /// \brief Dimensions of distance transform to fill
    ///@{
    imstkSetMacro(Dimensions, const Vec3i&);
    void setDimensions(int dimX, int dimY, int dimZ) { setDimensions(Vec3i(dimX, dimY, dimZ)); }
    imstkGetMacro(Dimensions, const Vec3i&);
    ///@}

    ///
    /// \brief Optionally one may specify bounds, if not specified
    /// bounds of the input SurfaceMesh is used
    ///
    /// note Vec6d is be of the format [minX, maxX, minY, maxY, minZ, maxZ]
    ///@{
    void setBounds(const Vec3d& min, const Vec3d& max);
    void setBounds(const Vec6d&);
    imstkGetMacro(Bounds, const Vec6d&);
    ///@}

    ///
    /// \brief The margin used when the bounds are not set explicitely
    ///
    imstkSetMacro(BoundsMargin, double);
    imstkGetMacro(BoundsMargin, double);

    ///
    /// \brief If on, will compute only a narrow banded transform
    ///@{
    imstkSetMacro(NarrowBanded, bool);
    imstkGetMacro(NarrowBanded, bool);
    ///@}

    ///
    /// \brief Width of the band
    ///@{
    imstkSetMacro(DilateSize, int);
    imstkGetMacro(DilateSize, int);
    ///@}

    ///@{
    imstkSetMacro(Tolerance, double);
    imstkGetMacro(Tolerance, double);
///@}

protected:
    void requestUpdate() override;

private:
    Vec3i  m_Dimensions = Vec3i::Zero();
    Vec6d  m_Bounds     = Vec6d::Zero();
    double m_Tolerance  = 1.0e-10;

    int  m_DilateSize   = 4; ///> Only for narrow banded
    bool m_NarrowBanded = false;

    double m_BoundsMargin = 10.0;
};
}