/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMeshIO.h"

struct aiMesh;

namespace imstk
{
///
/// \class AssimpMeshIO
///
/// \brief Assimp reader for surface meshes
///
class AssimpMeshIO
{
public:
    ///
    /// \brief Ensures file can be read and reads it if possible
    /// \param filePath path to file
    /// \param type mesh file type
    /// \returns iMSTK surface mesh
    ///
    static std::shared_ptr<PointSet> read(
        const std::string& filePath,
        MeshFileType       type);

    ///
    /// \brief Reads mesh data and returns mesh. May read a LineMesh
    /// if no triangles & only lines are present.
    /// \param filePath
    /// \returns iMSTK SurfaceMesh or LineMesh
    ///
    static std::shared_ptr<PointSet> readMeshData(const std::string& filePath);

    ///
    /// \brief Convert from Assimp mesh to iMSTK SurfaceMesh. May convert to LineMesh
    /// if no triangles & only lines are present
    /// \param importedMesh Assimp mesh to convert
    /// \returns iMSTK SurfaceMesh or LineMesh
    ///
    static std::shared_ptr<PointSet> convertAssimpMesh(aiMesh* importedMesh);

    ///
    /// \brief Helper function for getting default post processing flags
    /// \returns Bit mask of default flags
    /// This function should be used to ensure consistency among different loaders
    ///
    static unsigned int getDefaultPostProcessSteps();
};
} // namespace imstk
