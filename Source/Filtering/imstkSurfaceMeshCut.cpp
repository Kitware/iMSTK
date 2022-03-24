/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

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

#include "imstkSurfaceMeshCut.h"

#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkPlane.h"
#include "imstkAnalyticalGeometry.h"

#include <set>

namespace imstk
{
SurfaceMeshCut::SurfaceMeshCut()
{
    setNumInputPorts(1);
    setRequiredInputType<SurfaceMesh>(0);

    setNumOutputPorts(1);
    setOutput(std::make_shared<SurfaceMesh>());

    m_CutGeometry = std::make_shared<Plane>();
    m_RemoveConstraintVertices = std::make_shared<std::unordered_set<size_t>>();
    m_AddConstraintVertices    = std::make_shared<std::unordered_set<size_t>>();
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
SurfaceMeshCut::requestUpdate()
{
    // input and output SurfaceMesh
    std::shared_ptr<SurfaceMesh> inputSurf = std::dynamic_pointer_cast<SurfaceMesh>(getInput(0));
    if (inputSurf == nullptr)
    {
        LOG(WARNING) << "Missing required SurfaceMesh input";
        return;
    }
    std::shared_ptr<SurfaceMesh> outputSurf = std::dynamic_pointer_cast<SurfaceMesh>(getOutput(0));
    outputSurf->deepCopy(inputSurf);

    // vertices on the cutting path and whether they will be split
    std::map<int, bool> cutVerts;

    // generate cut data
    if (std::dynamic_pointer_cast<AnalyticalGeometry>(m_CutGeometry) != nullptr)
    {
        generateAnalyticalCutData(std::static_pointer_cast<AnalyticalGeometry>(m_CutGeometry), outputSurf);
    }
    else if (std::dynamic_pointer_cast<SurfaceMesh>(m_CutGeometry) != nullptr)
    {
        generateSurfaceMeshCutData(std::static_pointer_cast<SurfaceMesh>(m_CutGeometry), outputSurf);
    }
    else
    {
        setOutput(outputSurf);
        return;
    }

    if (m_CutData->size() == 0)
    {
        setOutput(outputSurf);
        return;
    }

    // refinement
    refinement(outputSurf, cutVerts);

    // split cutting vertices
    splitVerts(outputSurf, cutVerts, m_CutGeometry);

    // vtkPolyData to output SurfaceMesh
    setOutput(outputSurf);
}

void
SurfaceMeshCut::refinement(std::shared_ptr<SurfaceMesh> outputSurf, std::map<int, bool>& cutVerts)
{
    // map between one exsiting edge to the new vert generated from the cutting
    std::map<std::pair<int, int>, int> edgeVertMap;

    auto triangles = outputSurf->getTriangleIndices();
    auto vertices  = outputSurf->getVertexPositions();
    auto initVerts = outputSurf->getInitialVertexPositions();
    triangles->reserve(triangles->size() * 2);
    vertices->reserve(vertices->size() * 2);
    initVerts->reserve(initVerts->size() * 2);

    for (const auto& curCutData : *m_CutData)
    {
        auto  curCutType = curCutData.cutType;
        int   triId      = curCutData.triId;
        int   ptId0      = curCutData.ptIds[0];
        int   ptId1      = curCutData.ptIds[1];
        Vec3d cood0      = curCutData.cutCoords[0];
        Vec3d cood1      = curCutData.cutCoords[1];
        Vec3d initCood0  = curCutData.initCoords[0];
        Vec3d initCood1  = curCutData.initCoords[1];

        if (curCutType == CutType::EDGE || curCutType == CutType::EDGE_VERT)
        {
            int  newPtId = -1;
            auto edge0   = std::make_pair(ptId0, ptId1);
            auto edge1   = std::make_pair(ptId1, ptId0);

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
            Vec3i ptIds = (*triangles)[triId];
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
            (*triangles)[triId] = Vec3i(ptId2, ptId0, newPtId);
            triangles->push_back(Vec3i(ptId2, newPtId, ptId1));

            // add vertices to cutting path
            if (curCutType == CutType::EDGE_VERT)
            {
                cutVerts[ptId2]   = (cutVerts.find(ptId2) != cutVerts.end()) ? true : false;
                cutVerts[newPtId] = (cutVerts.find(newPtId) != cutVerts.end()) ? true : false;
            }

            m_RemoveConstraintVertices->insert(ptId0);
            m_RemoveConstraintVertices->insert(ptId1);
            m_RemoveConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(ptId0);
            m_AddConstraintVertices->insert(ptId1);
            m_AddConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(newPtId);
        }
        else if (curCutType == CutType::EDGE_EDGE)
        {
            int newPtId0 = -1;
            int newPtId1 = -1;

            Vec3i ptIds = (*triangles)[triId];
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

            auto edge00 = std::make_pair(ptId2, ptId0);
            auto edge01 = std::make_pair(ptId0, ptId2);
            auto edge10 = std::make_pair(ptId1, ptId2);
            auto edge11 = std::make_pair(ptId2, ptId1);

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
            (*triangles)[triId] = Vec3i(ptId2, newPtId0, newPtId1);
            triangles->push_back(Vec3i(newPtId0, ptId0, ptId1));
            triangles->push_back(Vec3i(newPtId0, ptId1, newPtId1));

            // add vertices to cutting path
            cutVerts[newPtId0] = (cutVerts.find(newPtId0) != cutVerts.end()) ? true : false;
            cutVerts[newPtId1] = (cutVerts.find(newPtId1) != cutVerts.end()) ? true : false;

            m_RemoveConstraintVertices->insert(ptId0);
            m_RemoveConstraintVertices->insert(ptId1);
            m_RemoveConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(ptId0);
            m_AddConstraintVertices->insert(ptId1);
            m_AddConstraintVertices->insert(ptId2);
            m_AddConstraintVertices->insert(newPtId0);
            m_AddConstraintVertices->insert(newPtId1);
        }
        else if (curCutType == CutType::VERT_VERT)
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

    outputSurf->modified();
}

void
SurfaceMeshCut::splitVerts(std::shared_ptr<SurfaceMesh> outputSurf, std::map<int, bool>& cutVerts, std::shared_ptr<Geometry> geometry)
{
    auto triangles = outputSurf->getTriangleIndices();
    auto vertices  = outputSurf->getVertexPositions();
    auto initVerts = outputSurf->getInitialVertexPositions();

    std::shared_ptr<AnalyticalGeometry> cutGeometry;
    if (std::dynamic_pointer_cast<AnalyticalGeometry>(geometry) != nullptr)
    {
        cutGeometry = std::static_pointer_cast<AnalyticalGeometry>(geometry);
    }
    else if (std::dynamic_pointer_cast<SurfaceMesh>(geometry) != nullptr)
    {
        // assuming triangles in cutSurf are co-planar
        auto cutSurf      = std::static_pointer_cast<SurfaceMesh>(geometry);
        auto cutTriangles = cutSurf->getTriangleIndices();
        auto cutVertices  = cutSurf->getVertexPositions();

        // compute cutting plane (assuming all triangles in cutSurf are co-planar)
        Vec3d p0 = (*cutVertices)[(*cutTriangles)[0][0]];
        Vec3d p1 = (*cutVertices)[(*cutTriangles)[0][1]];
        Vec3d p2 = (*cutVertices)[(*cutTriangles)[0][2]];
        Vec3d cutNormal = ((p1 - p0).cross(p2 - p0)).normalized();
        auto  cutPlane  = std::make_shared<Plane>(p0, cutNormal);
        cutGeometry = std::static_pointer_cast<AnalyticalGeometry>(cutPlane);
    }

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

                if (pointOnGeometrySide(pt0, cutGeometry) < 0
                    || pointOnGeometrySide(pt1, cutGeometry) < 0
                    || pointOnGeometrySide(pt2, cutGeometry) < 0)
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

    outputSurf->modified();
}

int
SurfaceMeshCut::pointOnGeometrySide(Vec3d pt, std::shared_ptr<Geometry> geometry)
{
    if (std::dynamic_pointer_cast<AnalyticalGeometry>(geometry) != nullptr)
    {
        return pointOnAnalyticalSide(pt, std::static_pointer_cast<AnalyticalGeometry>(geometry));
    }
    else if (std::dynamic_pointer_cast<SurfaceMesh>(geometry) != nullptr)
    {
        // save for curve surface cutting
    }
    return 0;
}

int
SurfaceMeshCut::pointOnAnalyticalSide(Vec3d pt, std::shared_ptr<AnalyticalGeometry> geometry)
{
    double normalProjection = geometry->getFunctionValue(pt);
    if (normalProjection > m_Epsilon)
    {
        return 1;
    }
    else if (normalProjection < -m_Epsilon)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    return 0;
}

bool
SurfaceMeshCut::vertexOnBoundary(std::shared_ptr<VecDataArray<int, 3>> triangleIndices, std::set<int>& triSet)
{
    std::set<int> nonRepeatNeighborVerts;
    for (const auto& tri : triSet)
    {
        for (int i = 0; i < 3; i++)
        {
            int ptId = (*triangleIndices)[tri][i];
            if (nonRepeatNeighborVerts.find(ptId) != nonRepeatNeighborVerts.end())
            {
                nonRepeatNeighborVerts.erase(ptId);
            }
            else
            {
                nonRepeatNeighborVerts.insert(ptId);
            }
        }
    }
    return (nonRepeatNeighborVerts.size() >= 2 ? true : false);
}

void
SurfaceMeshCut::generateAnalyticalCutData(std::shared_ptr<AnalyticalGeometry> geometry, std::shared_ptr<SurfaceMesh> outputSurf)
{
    auto triangles = outputSurf->getTriangleIndices();
    auto vertices  = outputSurf->getVertexPositions();
    auto initVerts = outputSurf->getInitialVertexPositions();

    m_CutData->clear();
    std::set<std::pair<int, int>> repeatEdges;  // make sure not boundary edge in vert-vert case

    for (int i = 0; i < triangles->size(); i++)
    {
        auto&   tri = (*triangles)[i];
        CutData newCutData;

        auto ptSide = Vec3i(pointOnAnalyticalSide((*vertices)[tri[0]], geometry),
                            pointOnAnalyticalSide((*vertices)[tri[1]], geometry),
                            pointOnAnalyticalSide((*vertices)[tri[2]], geometry));

        switch (ptSide.squaredNorm())
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
                        newCutData.cutType       = CutType::VERT_VERT;
                        newCutData.triId         = i;
                        newCutData.ptIds[0]      = ptId0;
                        newCutData.ptIds[1]      = ptId1;
                        newCutData.cutCoords[0]  = (*vertices)[ptId0];
                        newCutData.cutCoords[1]  = (*vertices)[ptId1];
                        newCutData.initCoords[0] = (*initVerts)[ptId0];
                        newCutData.initCoords[1] = (*initVerts)[ptId1];
                        m_CutData->push_back(newCutData);
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
                        double func0    = geometry->getFunctionValue(pos0);
                        double func1    = geometry->getFunctionValue(pos1);
                        double frac     = -func0 / (func1 - func0);

                        newCutData.cutType       = CutType::EDGE_VERT;
                        newCutData.triId         = i;
                        newCutData.ptIds[0]      = ptId0;
                        newCutData.ptIds[1]      = ptId1;
                        newCutData.cutCoords[0]  =  frac * (pos1 - pos0) + pos0;
                        newCutData.cutCoords[1]  = (*vertices)[tri[j]];
                        newCutData.initCoords[0] = frac * (initPos1 - initPos0) + initPos0;
                        newCutData.initCoords[1] = (*initVerts)[tri[j]];
                        m_CutData->push_back(newCutData);
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
                        double func0    = geometry->getFunctionValue(pos0);
                        double func1    = geometry->getFunctionValue(pos1);
                        double func2    = geometry->getFunctionValue(pos2);
                        double frac0    = -func0 / (func2 - func0);
                        double frac1    = -func1 / (func2 - func1);

                        newCutData.cutType       = CutType::EDGE_EDGE;
                        newCutData.triId         = i;
                        newCutData.ptIds[0]      = ptId0;
                        newCutData.ptIds[1]      = ptId1;
                        newCutData.cutCoords[0]  =  frac0 * (pos2 - pos0) + pos0;
                        newCutData.cutCoords[1]  =  frac1 * (pos2 - pos1) + pos1;
                        newCutData.initCoords[0] = frac0 * (initPos2 - initPos0) + initPos0;
                        newCutData.initCoords[1] = frac1 * (initPos2 - initPos1) + initPos1;
                        m_CutData->push_back(newCutData);
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
}

void
SurfaceMeshCut::generateSurfaceMeshCutData(std::shared_ptr<SurfaceMesh> cutSurf, std::shared_ptr<SurfaceMesh> outputSurf)
{
    auto cutTriangles = cutSurf->getTriangleIndices();
    auto cutVertices  = cutSurf->getVertexPositions();
    auto triangles    = outputSurf->getTriangleIndices();
    auto vertices     = outputSurf->getVertexPositions();

    // compute cutting plane (assuming all triangles in cutSurf are co-planar)
    Vec3d p0 = (*cutVertices)[(*cutTriangles)[0][0]];
    Vec3d p1 = (*cutVertices)[(*cutTriangles)[0][1]];
    Vec3d p2 = (*cutVertices)[(*cutTriangles)[0][2]];
    Vec3d cutNormal = (p1 - p0).cross(p2 - p0);
    auto  cutPlane  = std::make_shared<Plane>(p0, cutNormal);

    // Compute cut data using infinite cutPlane
    generateAnalyticalCutData(std::static_pointer_cast<AnalyticalGeometry>(cutPlane), outputSurf);

    // remove cutData that are out of the cutSurf
    std::shared_ptr<std::vector<CutData>> newCutData = std::make_shared<std::vector<CutData>>();
    for (const auto& curCutData : *m_CutData)
    {
        bool    coord0In      = pointProjectionInSurface(curCutData.cutCoords[0], cutSurf);
        bool    coord1In      = pointProjectionInSurface(curCutData.cutCoords[1], cutSurf);
        CutData newCurCutData = curCutData;

        switch (curCutData.cutType)
        {
        case CutType::VERT_VERT:
            if (coord0In && coord1In)
            {
                newCutData->push_back(newCurCutData);
            }
            break;
        case CutType::EDGE_VERT:
            if (coord0In)     // edge inside
            {
                if (coord1In) // vert inside
                {
                    newCutData->push_back(newCurCutData);
                }
                else
                {
                    newCurCutData.cutType = CutType::EDGE;
                    newCutData->push_back(newCurCutData);
                }
            }
            break;
        case CutType::EDGE_EDGE:
            if (coord0In)
            {
                if (coord1In)
                {
                    newCutData->push_back(newCurCutData);
                }
                else
                {
                    auto& tri = (*triangles)[newCurCutData.triId];
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
                    newCurCutData.cutType = CutType::EDGE;
                    newCutData->push_back(newCurCutData);
                }
            }
            else if (coord1In) // && coord0Out
            {
                auto& tri = (*triangles)[newCurCutData.triId];
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
                newCurCutData.cutType       = CutType::EDGE;
                newCutData->push_back(newCurCutData);
            }
            break;
        default:
            break;
        }
    }

    // update cutData
    m_CutData = newCutData;
}

bool
SurfaceMeshCut::pointProjectionInSurface(const Vec3d& pt, std::shared_ptr<SurfaceMesh> cutSurf)
{
    auto cutTriangles = cutSurf->getTriangleIndices();
    auto cutVertices  = cutSurf->getVertexPositions();
    bool inSurface    = false;

    for (const auto& tri : *cutTriangles)
    {
        Vec3d p0     = (*cutVertices)[tri[0]];
        Vec3d p1     = (*cutVertices)[tri[1]];
        Vec3d p2     = (*cutVertices)[tri[2]];
        Vec3d normal = (p1 - p0).cross(p2 - p0).normalized();

        double leftP0P1 = normal.dot((p1 - p0).cross(pt - p0));
        double leftP1P2 = normal.dot((p2 - p1).cross(pt - p1));
        double leftP2P0 = normal.dot((p0 - p2).cross(pt - p2));

        if (leftP0P1 >= 0 && leftP1P2 >= 0 && leftP2P0 >= 0)
        {
            inSurface = true;
            break;
        }
    }
    return inSurface;
}
} // namespace imstk
