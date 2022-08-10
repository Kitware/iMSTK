/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMeshCut.h"

namespace imstk
{
class ImplicitGeometry;
class LineMesh;
class SurfaceMesh;

enum class SegmentCutType
{
    NONE = 0,
    EDGE = 1 // Edge split
};

///
/// \class LineMeshCut
///
/// \brief This filter cuts the lines of a LineMesh into smaller
/// lines using input cutting geometry
/// Only supports convex shaped cutting, ie: An edge can't be
/// split twice
///
class LineMeshCut : public MeshCut
{
public:
    LineMeshCut();
    ~LineMeshCut() override = default;

    std::shared_ptr<LineMesh> getOutputMesh();
    void setInputMesh(std::shared_ptr<LineMesh> mesh);

protected:
    void refinement(std::shared_ptr<AbstractCellMesh> outputGeom,
                    std::map<int, bool>& cutVerts) override;
    void splitVerts(std::shared_ptr<AbstractCellMesh> outputGeom,
                    std::map<int, bool>& cutVerts,
                    std::shared_ptr<Geometry> cuttingGeom) override;

    std::shared_ptr<std::vector<CutData>> generateCutData(
        std::shared_ptr<Geometry>         cuttingGeom,
        std::shared_ptr<AbstractCellMesh> geomToCut) override;

    std::shared_ptr<std::vector<CutData>> generateImplicitCutData(
        std::shared_ptr<ImplicitGeometry> cuttingGeom,
        std::shared_ptr<LineMesh>         geomToCut);

    std::shared_ptr<std::vector<CutData>> generateSurfaceMeshCutData(
        std::shared_ptr<SurfaceMesh> cuttingGeom,
        std::shared_ptr<LineMesh>    geomToCut);
};
} // namespace imstk