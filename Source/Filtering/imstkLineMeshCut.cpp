/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkLineMeshCut.h"
#include "imstkImplicitGeometry.h"
#include "imstkLineMesh.h"
#include "imstkPlane.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
LineMeshCut::LineMeshCut()
{
    setNumOutputPorts(1);
    setOutput(std::make_shared<LineMesh>());
}

std::shared_ptr<LineMesh>
LineMeshCut::getOutputMesh()
{
    return std::dynamic_pointer_cast<LineMesh>(getOutput(0));
}

void
LineMeshCut::setInputMesh(std::shared_ptr<LineMesh> mesh)
{
    setInput(mesh, 0);
}

void
LineMeshCut::refinement(std::shared_ptr<AbstractCellMesh> outputGeom,
                        std::map<int, bool>& cutVerts)
{
    auto outputLineMesh = std::dynamic_pointer_cast<LineMesh>(outputGeom);

    std::shared_ptr<VecDataArray<int, 2>>    cells     = outputLineMesh->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices  = outputLineMesh->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> initVerts = outputLineMesh->getInitialVertexPositions();
    cells->reserve(cells->size() * 2);
    vertices->reserve(vertices->size() * 2);
    initVerts->reserve(initVerts->size() * 2);

    // Map between one exsiting edge to the new vert generated from the cutting
    std::map<std::pair<int, int>, int> edgeVertMap;
    for (const auto& curCutData : *m_CutData)
    {
        //const int curCutType = curCutData.cutType;
        const int    cellId = curCutData.cellId;
        const int    ptId0  = curCutData.ptIds[0];
        const int    ptId1  = curCutData.ptIds[1];
        const Vec3d& coord0 = curCutData.cutCoords[0];
        //const Vec3d& coord1 = curCutData.cutCoords[1];
        const Vec3d& initCoord0 = curCutData.initCoords[0];
        //const Vec3d& initCoord1 = curCutData.initCoords[1];

        // There is only one case just add a vertex from coord0
        vertices->push_back(coord0);
        initVerts->push_back(initCoord0);

        const int newPtId0 = vertices->size() - 1;

        // Rewire the edge from 1 to newPt
        // And newPt to 0
        const Vec2i prevCell = (*cells)[cellId];
        (*cells)[cellId] = Vec2i(prevCell[0], newPtId0);
        cells->push_back(Vec2i(newPtId0, prevCell[1]));

        // Add vertices to cutting path
        cutVerts[newPtId0] = true; // Split it

        // Regenerate constraints on these vertices
        m_RemoveConstraintVertices->insert(ptId0);
        m_RemoveConstraintVertices->insert(ptId1);
        m_AddConstraintVertices->insert(ptId0);
        m_AddConstraintVertices->insert(ptId1);
        m_AddConstraintVertices->insert(newPtId0);
    }
}

void
LineMeshCut::splitVerts(std::shared_ptr<AbstractCellMesh> outputGeom,
                        std::map<int, bool>& cutVerts,
                        std::shared_ptr<Geometry> cuttingGeom)
{
    auto outputLineMesh = std::dynamic_pointer_cast<LineMesh>(outputGeom);

    std::shared_ptr<VecDataArray<int, 2>>    cells     = outputLineMesh->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices  = outputLineMesh->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> initVerts = outputLineMesh->getInitialVertexPositions();

    std::shared_ptr<ImplicitGeometry> cutGeometry = nullptr;
    if (auto implicitCutGeom = std::dynamic_pointer_cast<ImplicitGeometry>(cuttingGeom))
    {
        cutGeometry = implicitCutGeom;
    }
    else if (auto surfMeshCutGeom = std::dynamic_pointer_cast<SurfaceMesh>(cuttingGeom))
    {
        // Assuming triangles in cutSurf are co-planar
        std::shared_ptr<VecDataArray<int, 3>>    cutTriangles = surfMeshCutGeom->getCells();
        std::shared_ptr<VecDataArray<double, 3>> cutVertices  = surfMeshCutGeom->getVertexPositions();

        // Compute cutting plane (assuming all triangles in cutSurf are co-planar)
        const Vec3d p0 = (*cutVertices)[(*cutTriangles)[0][0]];
        const Vec3d p1 = (*cutVertices)[(*cutTriangles)[0][1]];
        const Vec3d p2 = (*cutVertices)[(*cutTriangles)[0][2]];
        const Vec3d cutNormal = ((p1 - p0).cross(p2 - p0)).normalized();
        cutGeometry = std::make_shared<Plane>(p0, cutNormal);
    }
    CHECK(cutGeometry != nullptr) <<
        "Unsupported cut geometry. Only SurfaceMesh and ImplicitGeometry supported";

    // build vertexToTriangleListMap
    outputLineMesh->computeVertexToCellMap();
    const std::vector<std::unordered_set<int>>& vertexToCellMap = outputLineMesh->getVertexToCellMap();

    // Split cutting vertices (vertices on cut path)
    for (const auto& cutVert : cutVerts)
    {
        bool useFirstVertex = false;

        // For all cells connected to the cut vertex, make a new vertex to connect too
        for (const auto& cellId : vertexToCellMap[cutVert.first])
        {
            // The cut vertex should be re-used for at least/the first one of the cells
            if (!useFirstVertex)
            {
                useFirstVertex = true;
                continue;
            }

            // For every cell connected to the vertex to be split, make a duplicate
            // vertex with a newPtId, then rewire
            const int newPtId = vertices->size();
            vertices->push_back((*vertices)[cutVert.first]);
            initVerts->push_back((*initVerts)[cutVert.first]);
            (*m_CutVertMap)[cutVert.first] = newPtId;
            m_AddConstraintVertices->insert(newPtId);

            // Whichever vertex was pointing to the cut vertex, rewire
            Vec2i& cell = (*cells)[cellId];
            if (cell[0] == cutVert.first)
            {
                cell[0] = newPtId;
            }
            else if (cell[1] == cutVert.first)
            {
                cell[1] = newPtId;
            }
        }
    }
}

std::shared_ptr<std::vector<CutData>>
LineMeshCut::generateCutData(
    std::shared_ptr<Geometry>         cuttingGeom,
    std::shared_ptr<AbstractCellMesh> geomToCut)
{
    if (auto cuttingSurfMesh = std::dynamic_pointer_cast<SurfaceMesh>(cuttingGeom))
    {
        return generateSurfaceMeshCutData(cuttingSurfMesh,
            std::dynamic_pointer_cast<LineMesh>(geomToCut));
    }
    else if (auto cuttingAnalyticGeom = std::dynamic_pointer_cast<AnalyticalGeometry>(cuttingGeom))
    {
        return generateImplicitCutData(cuttingAnalyticGeom,
            std::dynamic_pointer_cast<LineMesh>(geomToCut));
    }
    LOG(FATAL) << "No case for cut geometry";
    return nullptr;
}

std::shared_ptr<std::vector<CutData>>
LineMeshCut::generateImplicitCutData(std::shared_ptr<ImplicitGeometry> cuttingGeom,
                                     std::shared_ptr<LineMesh>         geomToCut)
{
    auto cutData = std::make_shared<std::vector<CutData>>();

    auto                                     lineMeshToCut = std::dynamic_pointer_cast<LineMesh>(geomToCut);
    std::shared_ptr<VecDataArray<int, 2>>    cells     = lineMeshToCut->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices  = lineMeshToCut->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> initVerts = lineMeshToCut->getInitialVertexPositions();

    // For every edge/segment
    for (int i = 0; i < cells->size(); i++)
    {
        const Vec2i& cell = (*cells)[i];

        // Compute if cells vertices are inside or outside the geometry
        const Vec2i ptSide =
            Vec2i(ptBoundarySign((*vertices)[cell[0]], cuttingGeom),
                ptBoundarySign((*vertices)[cell[1]], cuttingGeom));

        // There is 3 cases
        // Both verts lie on pos side
        // Both verts lie on neg side
        // Verts lie on opposite sides

        // Both verts lie on the same side
        const int caseType = ptSide.squaredNorm();
        if (caseType != 2)
        {
            const int ptId0 = cell[0];
            const int ptId1 = cell[1];

            const Vec3d  pos0     = (*vertices)[ptId0];
            const Vec3d  pos1     = (*vertices)[ptId1];
            const Vec3d  initPos0 = (*initVerts)[ptId0];
            const Vec3d  initPos1 = (*initVerts)[ptId1];
            const double func0    = cuttingGeom->getFunctionValue(pos0);
            const double func1    = cuttingGeom->getFunctionValue(pos1);
            const double frac     = -func0 / (func1 - func0);
            const Vec3d  iPt      = frac * (pos1 - pos0) + pos0;
            const Vec3d  init_iPt = frac * (initPos1 - initPos0) + initPos0;

            {
                CutData cellCutData;
                cellCutData.cutType       = static_cast<int>(SegmentCutType::EDGE);
                cellCutData.cellId        = i;
                cellCutData.ptIds[0]      = ptId0;
                cellCutData.ptIds[1]      = ptId1;
                cellCutData.cutCoords[0]  = iPt;
                cellCutData.cutCoords[1]  = (*vertices)[ptId1];
                cellCutData.initCoords[0] = init_iPt;
                cellCutData.initCoords[1] = (*initVerts)[ptId1];
                cutData->push_back(cellCutData);
            }
        }
    }

    return cutData;
}

std::shared_ptr<std::vector<CutData>>
LineMeshCut::generateSurfaceMeshCutData(std::shared_ptr<SurfaceMesh> cuttingGeom,
                                        std::shared_ptr<LineMesh>    geomToCut)
{
    auto cutData = std::make_shared<std::vector<CutData>>();

    std::shared_ptr<VecDataArray<int, 3>>    cutTriangles = cuttingGeom->getCells();
    std::shared_ptr<VecDataArray<double, 3>> cutVertices  = cuttingGeom->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 2>>    lines    = geomToCut->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices = geomToCut->getVertexPositions();

    // compute cutting plane (assuming all triangles in cutSurf are co-planar)
    const Vec3d p0 = (*cutVertices)[(*cutTriangles)[0][0]];
    const Vec3d p1 = (*cutVertices)[(*cutTriangles)[0][1]];
    const Vec3d p2 = (*cutVertices)[(*cutTriangles)[0][2]];
    const Vec3d cutNormal = (p1 - p0).cross(p2 - p0);
    auto        cutPlane  = std::make_shared<Plane>(p0, cutNormal);

    // Compute cut data using infinite cutPlane
    std::shared_ptr<std::vector<CutData>> planeCutData =
        generateImplicitCutData(cutPlane, geomToCut);

    // Remove cutData that are out of the cutSurf
    for (const CutData& cellCutData : *planeCutData)
    {
        const bool coord0In = pointProjectionInSurface(cellCutData.cutCoords[0], cuttingGeom);
        const bool coord1In = pointProjectionInSurface(cellCutData.cutCoords[1], cuttingGeom);
        if (coord0In && coord1In)
        {
            cutData->push_back(cellCutData);
        }
    }

    return cutData;
}
} // namespace imstk