/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMath.h"
#include "imstkMeshIO.h"
#include "imstkVecDataArray.h"

namespace vega
{
class VolumetricMesh;
} // namespace vega

namespace imstk
{
template<typename T, int N> class VecDataArray;

///
/// \class VegaMeshIO
///
/// \brief Contains utility classes that convert vega volume mesh to volume mesh and
/// vice-versa
///
class VegaMeshIO
{
public:
    VegaMeshIO() = default;
    virtual ~VegaMeshIO() = default;

    ///
    /// \brief Read and generate volumetric mesh given a external vega mesh file
    ///
    static std::shared_ptr<PointSet> read(const std::string& filePath, MeshFileType meshType);

    ///
    /// \brief Read vega volume mesh from a file
    ///
    static std::shared_ptr<vega::VolumetricMesh> readVegaMesh(const std::string& filePath);

    ///
    /// \brief Write a volumetric mesh in vega file format
    ///
    static bool write(const std::shared_ptr<imstk::PointSet> imstkMesh, const std::string& filePath, const MeshFileType meshType);

    ///
    /// \brief Generate volumetric mesh given a vega volume mesh
    ///
    static std::shared_ptr<PointSet> convertVegaMeshToVolumetricMesh(std::shared_ptr<vega::VolumetricMesh> vegaVolumeMesh);

    ///
    /// \brief Generate a vega volume mesh given volumetric mesh
    ///
    static std::shared_ptr<vega::VolumetricMesh> convertVolumetricMeshToVegaMesh(const std::shared_ptr<PointSet> volumeMesh);

protected:
    ///
    /// \brief
    ///
    static void copyVertices(std::shared_ptr<vega::VolumetricMesh> vegaMesh, VecDataArray<double, 3>& vertices);

    ///
    /// \brief
    ///
    template<size_t dim>
    static void copyCells(std::shared_ptr<vega::VolumetricMesh> vegaMesh, VecDataArray<int, dim>& cells);
};
} // namespace imstk