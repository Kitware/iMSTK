/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSurfaceMeshCut.h"
#include "imstkAnalyticalGeometry.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkPlane.h"
#include "imstkSurfaceMesh.h"

#include <set>

namespace imstk
{
SurfaceMeshCut::SurfaceMeshCut()
{
    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());
}

std::shared_ptr<SurfaceMesh>
SurfaceMeshCut::getOutputMesh()
{
    return std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
}

void
SurfaceMeshCut::setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh)
{
    setInput(inputMesh, 0);
}

void
SurfaceMeshCut::refinement(std::shared_ptr<AbstractCellMesh> outputGeom,
                           std::map<int, bool>& cutVerts)
{
    auto outputSurfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(outputGeom);

    std::shared_ptr<VecDataArray<int, 3>>    cells     = outputSurfaceMesh->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices  = outputSurfaceMesh->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> initVerts = outputSurfaceMesh->getInitialVertexPositions();
    cells->reserve(cells->size() * 2);
    vertices->reserve(vertices->size() * 2);
    initVerts->reserve(initVerts->size() * 2);

    // map between one exsiting edge to the new vert generated from the cutting
    std::map<std::pair<int, int>, int> edgeVertMap;
    for (const auto& curCutData : *m_CutData)
    {
        const int    curCutType = curCutData.cutType;
        const int    triId      = curCutData.cellId;
        const int    ptId0      = curCutData.ptIds[0];
        const int    ptId1      = curCutData.ptIds[1];
        const Vec3d& cood0      = curCutData.cutCoords[0];
        const Vec3d& cood1      = curCutData.cutCoords[1];
        const Vec3d& initCood0  = curCutData.initCoords[0];
        const Vec3d& initCood1  = curCutData.initCoords[1];

        if (curCutType == static_cast<int>(TriCutType::EDGE)
            || curCutType == static_cast<int>(TriCutType::EDGE_VERT))
        {
            int  newPtId = -1;
            auto edge0   = std::pair<int, int>(ptId0, ptId1);
            auto edge1   = std::pair<int, int>(ptId1, ptId0);

            // add new point
            if (edgeVertMap.find(edge1) != edgeVertMap.end())
            {
                newPtId = edgeVertMap[edge1];
            }
            else
            {
                newPtId = vertices->size();
                vertices->push_back(cood0);
                initVerts->push_back(initCood0);
                edgeVertMap[edge0] = newPtId;
            }

            // update triangle indices
            Vec3i ptIds = (*cells)[triId];
            int   ptId2 = -1;
            if (ptIds[0] != ptId0 && ptIds[0] != ptId1)
            {
                ptId2 = ptIds[0];
            }
            else if (ptIds[1] != ptId0 && ptIds[1] != ptId1)
            {
                ptId2 = ptIds[1];
            }
            else
            {
                ptId2 = ptIds[2];
            }
            (*cells)[triId] = Vec3i(ptId2, ptId0, newPtId);
            cells->push_back(Vec3i(ptId2, newPtId, ptId1));

            // add vertices to cutting path
            if (curCutType == static_cast<int>(TriCutType::EDGE_VERT))
            {
                cutVerts[ptId2]   = (cutVerts.find(ptId2) != cutVerts.end());
                cutVerts[newPtId] = (cutVerts.find(newPtId) != cutVerts.end());
            }

            m_RemoveConstraintVertices->insert(ptId0);
            m_RemoveConstraintVertices->insert(ptId1);
            m_RemoveConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(ptId0);
            m_AddConstraintVertices->insert(ptId1);
            m_AddConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(newPtId);
        }
        else if (curCutType == static_cast<int>(TriCutType::EDGE_EDGE))
        {
            int newPtId0 = -1;
            int newPtId1 = -1;

            Vec3i ptIds = (*cells)[triId];
            int   ptId2 = -1;
            if (ptIds[0] != ptId0 && ptIds[0] != ptId1)
            {
                ptId2 = ptIds[0];
            }
            else if (ptIds[1] != ptId0 && ptIds[1] != ptId1)
            {
                ptId2 = ptIds[1];
            }
            else
            {
                ptId2 = ptIds[2];
            }

            auto edge00 = std::pair<int, int>(ptId2, ptId0);
            auto edge01 = std::pair<int, int>(ptId0, ptId2);
            auto edge10 = std::pair<int, int>(ptId1, ptId2);
            auto edge11 = std::pair<int, int>(ptId2, ptId1);

            // add new points
            if (edgeVertMap.find(edge01) != edgeVertMap.end())
            {
                newPtId0 = edgeVertMap[edge01];
            }
            else
            {
                newPtId0 = vertices->size();
                vertices->push_back(cood0);
                initVerts->push_back(initCood0);
                edgeVertMap[edge00] = newPtId0;
            }
            if (edgeVertMap.find(edge11) != edgeVertMap.end())
            {
                newPtId1 = edgeVertMap[edge11];
            }
            else
            {
                newPtId1 = vertices->size();
                vertices->push_back(cood1);
                initVerts->push_back(initCood1);
                edgeVertMap[edge10] = newPtId1;
            }

            // update triangle indices
            (*cells)[triId] = Vec3i(ptId2, newPtId0, newPtId1);
            cells->push_back(Vec3i(newPtId0, ptId0, ptId1));
            cells->push_back(Vec3i(newPtId0, ptId1, newPtId1));

            // add vertices to cutting path
            cutVerts[newPtId0] = (cutVerts.find(newPtId0) != cutVerts.end());
            cutVerts[newPtId1] = (cutVerts.find(newPtId1) != cutVerts.end());

            m_RemoveConstraintVertices->insert(ptId0);
            m_RemoveConstraintVertices->insert(ptId1);
            m_RemoveConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(ptId0);
            m_AddConstraintVertices->insert(ptId1);
            m_AddConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(newPtId0);
            m_AddConstraintVertices->insert(newPtId1);
        }
        else if (curCutType == static_cast<int>(TriCutType::VERT_VERT))
        {
            // add vertices to cutting path
            cutVerts[ptId0] = (cutVerts.find(ptId0) != cutVerts.end()) ? true : false;
            cutVerts[ptId1] = (cutVerts.find(ptId1) != cutVerts.end()) ? true : false;

            m_RemoveConstraintVertices->insert(ptId0);
            m_RemoveConstraintVertices->insert(ptId1);
            m_AddConstraintVertices->insert(ptId0);
            m_AddConstraintVertices->insert(ptId1);
        }
        else
        {
            //do nothing
        }
    }
}

void
SurfaceMeshCut::splitVerts(std::shared_ptr<AbstractCellMesh> outputGeom,
                           std::map<int, bool>& cutVerts, std::shared_ptr<Geometry> cuttingGeom)
{
    auto outputSurfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(outputGeom);

    std::shared_ptr<VecDataArray<int, 3>>    triangles = outputSurfaceMesh->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices  = outputSurfaceMesh->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> initVerts = outputSurfaceMesh->getInitialVertexPositions();

    std::shared_ptr<ImplicitGeometry> cutGeometry = nullptr;
    if (auto implicitCutGeom = std::dynamic_pointer_cast<ImplicitGeometry>(cuttingGeom))
    {
        cutGeometry = implicitCutGeom;
    }
    else if (auto surfMeshCutGeom = std::dynamic_pointer_cast<SurfaceMesh>(cuttingGeom))
    {
        // Assuming triangles in cutSurf are co-planar
        auto cutTriangles = surfMeshCutGeom->getCells();
        auto cutVertices  = surfMeshCutGeom->getVertexPositions();

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
    std::vector<std::set<int>> vertexNeighborTriangles;
    vertexNeighborTriangles.clear();
    vertexNeighborTriangles.resize(vertices->size());

    int triangleId = 0;
    for (const auto& tri : *triangles)
    {
        vertexNeighborTriangles.at(tri[0]).insert(triangleId);
        vertexNeighborTriangles.at(tri[1]).insert(triangleId);
        vertexNeighborTriangles.at(tri[2]).insert(triangleId);
        triangleId++;
    }

    // split cutting vertices
    for (const auto& cutVert : cutVerts)
    {
        if (cutVert.second == false && !vertexOnBoundary(triangles, vertexNeighborTriangles[cutVert.first]))
        {
            // do not split vertex since it's the cutting end in surface
            (*m_CutVertMap)[cutVert.first] = cutVert.first;
        }
        else
        {
            // split vertex
            int newPtId = vertices->size();
            vertices->push_back((*vertices)[cutVert.first]);
            initVerts->push_back((*initVerts)[cutVert.first]);
            (*m_CutVertMap)[cutVert.first] = newPtId;
            m_AddConstraintVertices->insert(newPtId);

            for (const auto& t : vertexNeighborTriangles[cutVert.first])
            {
                //if triangle on the negative side
                Vec3d pt0 = (*vertices)[(*triangles)[t][0]];
                Vec3d pt1 = (*vertices)[(*triangles)[t][1]];
                Vec3d pt2 = (*vertices)[(*triangles)[t][2]];

                if (ptBoundarySign(pt0, cutGeometry) < 0
                    || ptBoundarySign(pt1, cutGeometry) < 0
                    || ptBoundarySign(pt2, cutGeometry) < 0)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        if ((*triangles)[t][i] == cutVert.first)
                        {
                            (*triangles)[t][i] = newPtId;
                        }
                    }
                }
            }
        }
    }
}

std::shared_ptr<std::vector<CutData>>
SurfaceMeshCut::generateCutData(
    std::shared_ptr<Geometry>         cuttingGeom,
    std::shared_ptr<AbstractCellMesh> geomToCut)
{
    if (auto cuttingSurfMesh = std::dynamic_pointer_cast<SurfaceMesh>(cuttingGeom))
    {
        return generateSurfaceMeshCutData(cuttingSurfMesh,
            std::dynamic_pointer_cast<SurfaceMesh>(geomToCut));
    }
    else if (auto cuttingAnalyticGeom = std::dynamic_pointer_cast<AnalyticalGeometry>(cuttingGeom))
    {
        return generateImplicitCutData(cuttingAnalyticGeom,
            std::dynamic_pointer_cast<SurfaceMesh>(geomToCut));
    }
    LOG(FATAL) << "No case for cut geometry";
    return nullptr;
}

std::shared_ptr<std::vector<CutData>>
SurfaceMeshCut::generateImplicitCutData(std::shared_ptr<AnalyticalGeometry> cuttingGeom,
                                        std::shared_ptr<SurfaceMesh>        geomToCut)
{
    auto cutData = std::make_shared<std::vector<CutData>>();

    std::shared_ptr<VecDataArray<int, 3>>    triangles = geomToCut->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices  = geomToCut->getVertexPositions();
    std::shared_ptr<VecDataArray<double, 3>> initVerts = geomToCut->getInitialVertexPositions();

    std::set<std::pair<int, int>> repeatEdges;  // make sure not boundary edge in vert-vert case

    for (int i = 0; i < triangles->size(); i++)
    {
        const Vec3i& tri = (*triangles)[i];

        // Compute if triangles vertex is inside or outside the geometry
        const Vec3i ptSide =
            Vec3i(ptBoundarySign((*vertices)[tri[0]], cuttingGeom),
                ptBoundarySign((*vertices)[tri[1]], cuttingGeom),
                ptBoundarySign((*vertices)[tri[2]], cuttingGeom));

        // Square norm removes negatives
        // Produces differing outputs when some vertices
        // are on the border
        const int caseType = ptSide.squaredNorm();

        CutData newCutData;
        switch (caseType)
        {
        case 1:
            for (int j = 0; j < 3; j++)
            {
                if (ptSide[j] != 0)
                {
                    int idx0  = (j + 1) % 3;
                    int idx1  = (j + 2) % 3;
                    int ptId0 = tri[idx0];
                    int ptId1 = tri[idx1];

                    std::pair<int, int> cutEdge = std::make_pair(ptId1, ptId0);
                    // if find cut triangle from the other side of the edge, add newCutData
                    if (repeatEdges.find(cutEdge) != repeatEdges.end())
                    {
                        newCutData.cutType       = static_cast<int>(TriCutType::VERT_VERT);
                        newCutData.cellId        = i;
                        newCutData.ptIds[0]      = ptId0;
                        newCutData.ptIds[1]      = ptId1;
                        newCutData.cutCoords[0]  = (*vertices)[ptId0];
                        newCutData.cutCoords[1]  = (*vertices)[ptId1];
                        newCutData.initCoords[0] = (*initVerts)[ptId0];
                        newCutData.initCoords[1] = (*initVerts)[ptId1];
                        cutData->push_back(newCutData);
                    }
                    else
                    {
                        repeatEdges.insert(std::make_pair(ptId0, ptId1));
                    }
                }
            }
            break;
        case 2:
            if (ptSide.sum() == 0)
            {
                for (int j = 0; j < 3; j++)
                {
                    if (ptSide[j] == 0)
                    {
                        int    idx0     = (j + 1) % 3;
                        int    idx1     = (j + 2) % 3;
                        int    ptId0    = tri[idx0];
                        int    ptId1    = tri[idx1];
                        Vec3d  pos0     = (*vertices)[ptId0];
                        Vec3d  pos1     = (*vertices)[ptId1];
                        Vec3d  initPos0 = (*initVerts)[ptId0];
                        Vec3d  initPos1 = (*initVerts)[ptId1];
                        double func0    = cuttingGeom->getFunctionValue(pos0);
                        double func1    = cuttingGeom->getFunctionValue(pos1);
                        double frac     = -func0 / (func1 - func0);

                        newCutData.cutType       = static_cast<int>(TriCutType::EDGE_VERT);
                        newCutData.cellId        = i;
                        newCutData.ptIds[0]      = ptId0;
                        newCutData.ptIds[1]      = ptId1;
                        newCutData.cutCoords[0]  =  frac * (pos1 - pos0) + pos0;
                        newCutData.cutCoords[1]  = (*vertices)[tri[j]];
                        newCutData.initCoords[0] = frac * (initPos1 - initPos0) + initPos0;
                        newCutData.initCoords[1] = (*initVerts)[tri[j]];
                        cutData->push_back(newCutData);
                    }
                }
            }
            else
            {
                // newCutData.cutType = CutType::VERT;
            }
            break;
        case 3:
            if (ptSide.sum() == -1 || ptSide.sum() == 1)
            {
                for (int j = 0; j < 3; j++)
                {
                    if (ptSide[j] == -ptSide.sum())
                    {
                        int    idx0     = (j + 1) % 3;
                        int    idx1     = (j + 2) % 3;
                        int    ptId0    = tri[idx0];
                        int    ptId1    = tri[idx1];
                        int    ptId2    = tri[j];
                        Vec3d  pos0     = (*vertices)[ptId0];
                        Vec3d  pos1     = (*vertices)[ptId1];
                        Vec3d  pos2     = (*vertices)[ptId2];
                        Vec3d  initPos0 = (*initVerts)[ptId0];
                        Vec3d  initPos1 = (*initVerts)[ptId1];
                        Vec3d  initPos2 = (*initVerts)[ptId2];
                        double func0    = cuttingGeom->getFunctionValue(pos0);
                        double func1    = cuttingGeom->getFunctionValue(pos1);
                        double func2    = cuttingGeom->getFunctionValue(pos2);
                        double frac0    = -func0 / (func2 - func0);
                        double frac1    = -func1 / (func2 - func1);

                        newCutData.cutType       = static_cast<int>(TriCutType::EDGE_EDGE);
                        newCutData.cellId        = i;
                        newCutData.ptIds[0]      = ptId0;
                        newCutData.ptIds[1]      = ptId1;
                        newCutData.cutCoords[0]  =  frac0 * (pos2 - pos0) + pos0;
                        newCutData.cutCoords[1]  =  frac1 * (pos2 - pos1) + pos1;
                        newCutData.initCoords[0] = frac0 * (initPos2 - initPos0) + initPos0;
                        newCutData.initCoords[1] = frac1 * (initPos2 - initPos1) + initPos1;
                        cutData->push_back(newCutData);
                    }
                }
            }
            else
            {
                // no intersection
            }
            break;
        default:
            break;
        }
    }
    return cutData;
}

std::shared_ptr<std::vector<CutData>>
SurfaceMeshCut::generateSurfaceMeshCutData(std::shared_ptr<SurfaceMesh> cuttingGeom,
                                           std::shared_ptr<SurfaceMesh> geomToCut)
{
    auto cutData = std::make_shared<std::vector<CutData>>();

    std::shared_ptr<VecDataArray<int, 3>>    cutTriangles = cuttingGeom->getCells();
    std::shared_ptr<VecDataArray<double, 3>> cutVertices  = cuttingGeom->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 3>>    triangles    = geomToCut->getCells();
    std::shared_ptr<VecDataArray<double, 3>> vertices     = geomToCut->getVertexPositions();

    // compute cutting plane (assuming all triangles in cutSurf are co-planar)
    Vec3d p0 = (*cutVertices)[(*cutTriangles)[0][0]];
    Vec3d p1 = (*cutVertices)[(*cutTriangles)[0][1]];
    Vec3d p2 = (*cutVertices)[(*cutTriangles)[0][2]];
    Vec3d cutNormal = (p1 - p0).cross(p2 - p0);
    auto  cutPlane  = std::make_shared<Plane>(p0, cutNormal);

    // Compute cut data using infinite cutPlane
    std::shared_ptr<std::vector<CutData>> planeCutData =
        generateImplicitCutData(std::dynamic_pointer_cast<AnalyticalGeometry>(cutPlane), geomToCut);

    // Remove cutData that are out of the cutSurf
    for (const CutData& curCutData : *planeCutData)
    {
        bool    coord0In      = pointProjectionInSurface(curCutData.cutCoords[0], cuttingGeom);
        bool    coord1In      = pointProjectionInSurface(curCutData.cutCoords[1], cuttingGeom);
        CutData newCurCutData = curCutData;

        switch (curCutData.cutType)
        {
        case static_cast<int>(TriCutType::VERT_VERT):
            if (coord0In && coord1In)
            {
                cutData->push_back(newCurCutData);
            }
            break;
        case static_cast<int>(TriCutType::EDGE_VERT):
            if (coord0In)     // edge inside
            {
                if (coord1In) // vert inside
                {
                    cutData->push_back(newCurCutData);
                }
                else
                {
                    newCurCutData.cutType = static_cast<int>(TriCutType::EDGE);
                    cutData->push_back(newCurCutData);
                }
            }
            break;
        case static_cast<int>(TriCutType::EDGE_EDGE):
            if (coord0In)
            {
                if (coord1In)
                {
                    cutData->push_back(newCurCutData);
                }
                else
                {
                    auto& tri = (*triangles)[newCurCutData.cellId];
                    for (int i = 0; i < 3; i++)
                    {
                        if (tri[i] == newCurCutData.ptIds[0])
                        {
                            int idx0 = (i + 2) % 3;
                            newCurCutData.ptIds[0] = tri[idx0];
                            newCurCutData.ptIds[1] = tri[i];
                            break;
                        }
                    }
                    newCurCutData.cutType = static_cast<int>(TriCutType::EDGE);
                    cutData->push_back(newCurCutData);
                }
            }
            else if (coord1In) // && coord0Out
            {
                auto& tri = (*triangles)[newCurCutData.cellId];
                for (int i = 0; i < 3; i++)
                {
                    if (tri[i] == curCutData.ptIds[0])
                    {
                        int idx0 = (i + 1) % 3;
                        int idx1 = (i + 2) % 3;
                        newCurCutData.ptIds[0] = tri[idx0];
                        newCurCutData.ptIds[1] = tri[idx1];
                        break;
                    }
                }
                newCurCutData.cutCoords[0]  = newCurCutData.cutCoords[1];
                newCurCutData.initCoords[0] = newCurCutData.initCoords[1];
                newCurCutData.cutType       = static_cast<int>(TriCutType::EDGE);
                cutData->push_back(newCurCutData);
            }
            break;
        default:
            break;
        }
    }

    return cutData;
}
} // namespace imstk
