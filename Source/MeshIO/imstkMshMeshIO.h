/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMeshIO.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class MshMeshIO
///
/// \brief Can read/return LineMesh, SurfaceMesh, TetrahedralMesh, or
/// HexahedralMesh from given .msh file. Can only read homogenous elements.
///
/// If given a file with mixed elements it will always choose the elements
/// with the most vertices. Ex: Given both tetrahedral and triangle data,
/// only the tetrahedral will be read.
///
/// Only supports vertex data that is doubles (8 byte sized floating point).
///
class MshMeshIO
{
public:
    MshMeshIO() = default;
    virtual ~MshMeshIO() = default;

    ///
    /// \brief Read and generate a volumetric mesh given a external msh file
    ///
    static std::shared_ptr<PointSet> read(const std::string& filePath);
};
} // namespace imstk
