/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    ~SurfaceMeshImageMask() override = default;

public:
    ///
    /// \brief Optional input, used for information (dimensions, spacing, etc)
    ///
    void setReferenceImage(std::shared_ptr<ImageData> refImage);
    void setInputMesh(std::shared_ptr<SurfaceMesh> mesh);
    std::shared_ptr<ImageData> getOutputImage() const;

    ///
    /// \brief Get/Set a border around the generated image of pixel amount
    /// border exists outside of bounds
    ///@{
    imstkSetMacro(BorderExtent, int);
    imstkGetMacro(BorderExtent, int);
    ///@}

    ///
    /// \brief The dimensions of the image to generate
    ///@{
    imstkSetMacro(Dimensions, const Vec3i&);
    imstkGetMacro(Dimensions, const Vec3i&);
    ///@}

    ///
    /// \brief The dimensions of the image to generate
    ///
    void setDimensions(int dimX, int dimY, int dimZ) { setDimensions(Vec3i(dimX, dimY, dimZ)); }

protected:
    void requestUpdate() override;

private:
    Vec3i m_Dimensions    = Vec3i(-1, -1, -1);
    int   m_BorderExtent  = 1;
    bool  m_CheckManifold = true;
};
} // namespace imstk