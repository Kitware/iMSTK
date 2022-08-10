/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMeshCut.h"

namespace imstk
{
// vertex on the plane (0), positive side (+1), negative side (-1)
// pt0 and pt1 follows the triangle's indexing order when tri is presented
// c0 and c1 are cutting coordinates stored in cutData
enum class TriCutType
{
    NONE = 0,
    /* triangle is not cut through
    *       pt0 (-+1)
    *           /  \
    *       c0 /    \
    *         / tri  \
    * pt1 (+-1)------(?)
    */
    EDGE,
    /*
    *      (-+1)
    *       /  \
    *      /    \
    *     / tri  \
    *  (-+1)------(0) pt0/c0
    */
    VERT,
    /*
    *        (+-1) pt1
    *        /  \
    *    c1 /    \
    *      / tri  \
    *  (-+1)--c0--(+-1) pt0
    */
    EDGE_EDGE,
    /*
    *        pt0 (+-1)
    *            /  \
    *        c0 /    \
    *          / tri  \
    *  pt1 (-+1)------(0) c1
    */
    EDGE_VERT,
    /*
    * pt0/c0 (0)------(+-1)
    *        /  \      /
    *       /    \    /
    *      /      \  /
    *   (-+1)------(0) pt1/c1
    */
    VERT_VERT
};

///
/// \class SurfaceMeshCut
///
/// \brief This filter cuts the triangles of a SurfaceMesh into smaller
/// triangles using input cutting geometry
///
class SurfaceMeshCut : public MeshCut
{
public:
    SurfaceMeshCut();
    ~SurfaceMeshCut() override = default;

    std::shared_ptr<SurfaceMesh> getOutputMesh();
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputSurf);

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
        std::shared_ptr<AnalyticalGeometry> cuttingGeom,
        std::shared_ptr<SurfaceMesh>        geomToCut);

    std::shared_ptr<std::vector<CutData>> generateSurfaceMeshCutData(
        std::shared_ptr<SurfaceMesh> cuttingGeom,
        std::shared_ptr<SurfaceMesh> geomToCut);
};
} // namespace imstk