/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkMath.h"

#include <vtkSmartPointer.h>

class vtkImplicitPolyDataDistance;

namespace imstk
{
class ImageData;
class SurfaceMesh;

///
/// \class SurfaceMeshDistanceTransform
///
/// \brief This filter computes exact signed distance fields using octrees
/// and pseudonormal computations. One might need to adjust the tolerance
/// depending on dataset scale.
/// The bounds for the image can be set in the filter, when none are set
/// the bounding box of the mesh is used, the margin.  When providing your own bounds a
/// box larger than the original object might be necessary depending on shape
class SurfaceMeshDistanceTransform : public GeometryAlgorithm
{
public:
    SurfaceMeshDistanceTransform();
    ~SurfaceMeshDistanceTransform() override = default;

public:
    ///
    /// \brief Required input, port 0
    ///
    void setInputMesh(std::shared_ptr<SurfaceMesh> surfMesh);

    std::shared_ptr<ImageData> getOutputImage();

    void setupDistFunc();

    ///
    /// \brief Get the nearest point
    ///
    Vec3d getNearestPoint(const Vec3d& pos);

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

    bool m_NarrowBanded = false;
    int  m_DilateSize   = 4; ///< Only for narrow banded

    vtkSmartPointer<vtkImplicitPolyDataDistance> m_distFunc = nullptr;
};
} // namespace imstk