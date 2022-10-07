/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkProximitySurfaceSelector.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

namespace imstk
{
ProximitySurfaceSelector::ProximitySurfaceSelector()
{
    setNumInputPorts(2);
    setRequiredInputType<SurfaceMesh>(0);
    setRequiredInputType<SurfaceMesh>(1);

    setNumOutputPorts(2);
    setOutput(std::shared_ptr<SurfaceMesh>(), 0);
    setOutput(std::shared_ptr<SurfaceMesh>(), 1);
}

void
ProximitySurfaceSelector::setInputMeshes(
    std::shared_ptr<SurfaceMesh> inputMeshA,
    std::shared_ptr<SurfaceMesh> inputMeshB)
{
    setInput(inputMeshA, 0);
    setInput(inputMeshB, 1);
}

std::shared_ptr<SurfaceMesh>
ProximitySurfaceSelector::getOutputMeshA() const
{
    return std::static_pointer_cast<SurfaceMesh>(getOutput(0));
}

std::shared_ptr<SurfaceMesh>
ProximitySurfaceSelector::getOutputMeshB() const
{
    return std::static_pointer_cast<SurfaceMesh>(getOutput(1));
}

void
ProximitySurfaceSelector::requestUpdate()
{
    std::shared_ptr<SurfaceMesh> meshA = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    std::shared_ptr<SurfaceMesh> meshB = std::dynamic_pointer_cast<SurfaceMesh>(getInput(1));

    // Handy storage for tracking which faces have been added
    std::pair<std::vector<int>, std::vector<int>> closeSurfaces;

    // Handy storage for tracking and generating sub surface meshes
    std::pair<std::shared_ptr<SurfaceMesh>, std::shared_ptr<SurfaceMesh>> subMeshes;

    subMeshes.first  = std::make_shared<SurfaceMesh>();
    subMeshes.second = std::make_shared<SurfaceMesh>();

    // Check minimum distance
    double minDist = IMSTK_DOUBLE_MAX;
    for (int vertId_a = 0; vertId_a < meshA->getNumVertices(); vertId_a++)
    {
        for (int vertId_b = 0; vertId_b < meshB->getNumVertices(); vertId_b++)
        {
            const auto& vertA = meshA->getVertexPosition(vertId_a);
            const auto& vertB = meshB->getVertexPosition(vertId_b);
            minDist = std::min(minDist, (vertA - vertB).norm());
        }
    }

    if (minDist > m_proximity)
    {
        LOG(WARNING) << "No SurfaceMeshes generated, the meshes are further apart than the requested proximity";
        return;
    }

    // Unpack cell and vertex data for meshA
    std::shared_ptr<VecDataArray<int, 3>>    meshACellsPtr = meshA->getCells();
    VecDataArray<int, 3>&                    meshACells    = *meshACellsPtr;
    std::shared_ptr<VecDataArray<double, 3>> meshAVertsPtr = meshA->getVertexPositions();
    VecDataArray<double, 3>&                 meshAVerts    = *meshAVertsPtr;

    // Unpack cell and vertex data for meshB
    std::shared_ptr<VecDataArray<int, 3>>    meshBCellsPtr = meshB->getCells();
    VecDataArray<int, 3>&                    meshBCells    = *meshBCellsPtr;
    std::shared_ptr<VecDataArray<double, 3>> meshBVertsPtr = meshB->getVertexPositions();
    VecDataArray<double, 3>&                 meshBVerts    = *meshBVertsPtr;

    // Check if the center of a triangle on Mesh A is within maxDistance of any
    // triangle center on mesh B.  If so, add to list. Also, vice versa.

    // Storage for vertex indices of sub triangle
    auto                  subIndicesPtrA = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>& subIndicesA    = *subIndicesPtrA;

    auto                  subIndicesPtrB = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>& subIndicesB    = *subIndicesPtrB;

    for (int cellId_a = 0; cellId_a < meshACells.size(); cellId_a++)
    {
        // Calculate position of center of triangle
        const Vec3i& triangleVertexIdsA = meshACells[cellId_a];
        Vec3d        cellACenter = (meshA->getVertexPosition(triangleVertexIdsA(0))
                                    + meshA->getVertexPosition(triangleVertexIdsA(1))
                                    + meshA->getVertexPosition(triangleVertexIdsA(2))) / 3.0;

        // Check if within maxDist of any cell on B
        for (int cellId_b = 0; cellId_b < meshBCells.size(); cellId_b++)
        {
            const Vec3i& triangleVertexIdsB = meshBCells[cellId_b];
            Vec3d        cellBCenter = (meshB->getVertexPosition(triangleVertexIdsB(0))
                                        + meshB->getVertexPosition(triangleVertexIdsB(1))
                                        + meshB->getVertexPosition(triangleVertexIdsB(2))) / 3.0;

            if ((cellACenter - cellBCenter).squaredNorm() <= m_proximity * m_proximity)
            {
                // If this surface has already been added, skip
                if (std::find(closeSurfaces.first.begin(), closeSurfaces.first.end(), cellId_a) == closeSurfaces.first.end())
                {
                    closeSurfaces.first.push_back(cellId_a);
                    // If this triangle is within maxDist of Mesh B, add its vertex indiecs to subset
                    const Vec3i& triangleVertexIds = meshACells[cellId_a];
                    subIndicesA.push_back(triangleVertexIds);
                }
                if (std::find(closeSurfaces.second.begin(), closeSurfaces.second.end(), cellId_b) == closeSurfaces.second.end())
                {
                    closeSurfaces.second.push_back(cellId_b);
                    // If this triangle is within maxDist of Mesh B, add its vertex indiecs to subset
                    const Vec3i& triangleVertexIds = meshBCells[cellId_b];
                    subIndicesB.push_back(triangleVertexIds);
                }
            }
        }
    }

    // Initialize submesh from mesh A
    subMeshes.first->initialize(meshAVertsPtr, subIndicesPtrA);
    setOutput(subMeshes.first, 0);

    // Initialize submesh from mesh B
    subMeshes.second->initialize(meshBVertsPtr, subIndicesPtrB);
    setOutput(subMeshes.second, 1);
}
} // namespace imstk