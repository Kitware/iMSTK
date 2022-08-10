/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkMeshCut.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
MeshCut::MeshCut()
{
    setNumInputPorts(1);
    setRequiredInputType<AbstractCellMesh>(0);

    m_CutGeometry = nullptr;
    m_RemoveConstraintVertices = std::make_shared<std::unordered_set<size_t>>();
    m_AddConstraintVertices    = std::make_shared<std::unordered_set<size_t>>();
}

void
MeshCut::requestUpdate()
{
    // input and output SurfaceMesh
    auto inputGeom = std::dynamic_pointer_cast<AbstractCellMesh>(getInput(0));
    if (inputGeom == nullptr)
    {
        LOG(WARNING) << "Missing required AbstractCellMesh input";
        return;
    }
    // Copy input to output
    auto outputGeom = std::dynamic_pointer_cast<AbstractCellMesh>(getOutput(0));
    if (auto outputLineMesh = std::dynamic_pointer_cast<CellMesh<2>>(outputGeom))
    {
        outputLineMesh->deepCopy(std::dynamic_pointer_cast<CellMesh<2>>(inputGeom));
    }
    else if (auto outputTriMesh = std::dynamic_pointer_cast<CellMesh<3>>(outputGeom))
    {
        outputTriMesh->deepCopy(std::dynamic_pointer_cast<CellMesh<3>>(inputGeom));
    }
    setOutput(outputGeom);

    // Vertices on the cutting path and whether they will be split
    std::map<int, bool> cutVerts;

    // Compute the CutData which defines how to perform the cut
    m_CutData = generateCutData(m_CutGeometry, outputGeom);
    if (m_CutData->size() == 0)
    {
        return;
    }

    // Refine the mesh, adding vertices where the cutting occurs
    refinement(outputGeom, cutVerts);

    // Split cutting vertices, separating the geometry
    splitVerts(outputGeom, cutVerts, m_CutGeometry);
}

int
MeshCut::ptBoundarySign(const Vec3d& pt, std::shared_ptr<Geometry> geometry)
{
    if (auto implicitGeom = std::dynamic_pointer_cast<ImplicitGeometry>(geometry))
    {
        const double normalProjection = implicitGeom->getFunctionValue(pt);
        if (normalProjection > m_Epsilon)
        {
            return 1;
        }
        else if (normalProjection < -m_Epsilon)
        {
            return -1;
        }
    }
    else if (std::dynamic_pointer_cast<SurfaceMesh>(geometry) != nullptr)
    {
        // save for curve surface cutting
    }
    return 0;
}

bool
MeshCut::pointProjectionInSurface(const Vec3d&                 pt,
                                  std::shared_ptr<SurfaceMesh> surface)
{
    std::shared_ptr<VecDataArray<int, 3>>    triangles = surface->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices  = surface->getVertexPositions();
    bool                                     inSurface = false;

    for (const Vec3i& tri : *triangles)
    {
        const Vec3d p0     = (*vertices)[tri[0]];
        const Vec3d p1     = (*vertices)[tri[1]];
        const Vec3d p2     = (*vertices)[tri[2]];
        const Vec3d normal = (p1 - p0).cross(p2 - p0).normalized();

        const double leftP0P1 = normal.dot((p1 - p0).cross(pt - p0));
        const double leftP1P2 = normal.dot((p2 - p1).cross(pt - p1));
        const double leftP2P0 = normal.dot((p0 - p2).cross(pt - p2));

        if (leftP0P1 >= 0.0 && leftP1P2 >= 0.0 && leftP2P0 >= 0.0)
        {
            inSurface = true;
            break;
        }
    }
    return inSurface;
}
} // namespace imstk