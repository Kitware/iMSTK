/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkMath.h"

#include <list>
#include <unordered_set>

namespace imstk
{
class ImageData;
class SurfaceMesh;

///
/// \class LocalMarchingCubes
///
/// \brief This filter extracts a contour SurfaceMesh from an image given an
/// isovalue. Users should prefer imstkFlyingEdges over this unless sparse/local
/// processing is needed
///
/// One can provide this filter with a list of voxels that were modified in
/// the image and it will locally update it such that little processing is done
///
/// It works in chunks, so a set of SurfaceMesh's are the output. One can provide
/// the filter with the number of divisions on each axes to split up the image
///
class LocalMarchingCubes : public GeometryAlgorithm
{
public:
    LocalMarchingCubes();
    ~LocalMarchingCubes() override = default;

    std::shared_ptr<SurfaceMesh> getOutputMesh(const int i) const;

    void setInputImage(std::shared_ptr<ImageData> inputImage);

    ///
    /// \brief Value where the boundary lies
    ///
    void setIsoValue(double isovalue) { m_isoValue = isovalue; }

    ///
    /// \brief Set all voxels as modified (used initially)
    ///
    void setAllModified(bool val) { m_allModified = val; }

    ///
    /// \brief Set a voxel that was modified in the image (the neighboring dual
    /// voxels will be updated on the next request)
    ///
    void setModified(const Vec3i& coord);

    ///
    /// \brief Clear all pending modifications
    ///
    void clearModified() { m_modifiedVoxels.clear(); }

    ///
    /// \brief Set the number of chunks.
    /// one minus the dimensions of image must be divisible by numChunks
    /// ((dimensionX - 1) / numChunks must have no remainder)
    /// If it doesn't the *next* divisor will be used, warning thrown
    ///
    void setNumberOfChunks(const Vec3i& numChunks);

    const Vec3i& getNumberOfChunks() const { return m_numChunks; }

    std::unordered_map<int, Vec3i> getModifiedChunks() { return m_modifiedChunks; }

protected:
    void requestUpdate() override;

private:
    // Id + coordinate of the modified voxels
    std::unordered_map<int, Vec3i> m_modifiedVoxels;   // Id + coordinate

    bool m_allModified = true;

    double m_isoValue = 0.0;

    Vec3i  m_numChunks  = Vec3i(0, 0, 0); // Number of chunks along x, y, z
    size_t m_chunkCount = 0;              ///< Total chunk count x * y * z

    std::unordered_map<int, Vec3i> m_modifiedChunks;
};
} // namespace imstk