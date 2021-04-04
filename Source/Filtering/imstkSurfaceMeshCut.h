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
#pragma once

#include "imstkGeometryAlgorithm.h"
#include "imstkSurfaceMesh.h"
#include <map>
#include <unordered_set>

namespace imstk
{
class Geometry;
class AnalyticalGeometry;

// vertex on the plane (0), positive side (+1), negative side (-1)
// pt0 and pt1 follows the triangle's indexing order when tri is presented
// c0 and c1 are cutting coordinates stored in cutData
enum class CutType
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

struct CutData
{
    public:
        Vec3d cutCoords[2];
        Vec3d initCoords[2];
        int triId       = -1;
        int ptIds[2]    = { -1, -1 };
        CutType cutType = CutType::NONE;
};

///
/// \class SurfaceMeshCut
///
/// \brief This filter cuts the triangles of a SurfaceMesh into smaller
/// triangles using input cutData
/// \todo: test
///
class SurfaceMeshCut : public GeometryAlgorithm
{
public:
    SurfaceMeshCut();
    virtual ~SurfaceMeshCut() override = default;

public:
    std::shared_ptr<SurfaceMesh> getOutputMesh();
    void setInputMesh(std::shared_ptr<SurfaceMesh> inputSurf);
    std::shared_ptr<std::map<int, int>> getCutVertMap() { return m_CutVertMap; }

    imstkGetMacro(CutData, std::shared_ptr<std::vector<CutData>>);
    imstkSetMacro(CutData, std::shared_ptr<std::vector<CutData>>);
    imstkGetMacro(CutGeometry, std::shared_ptr<Geometry>);
    imstkSetMacro(CutGeometry, std::shared_ptr<Geometry>);
    imstkGetMacro(Epsilon, double);
    imstkSetMacro(Epsilon, double);
    imstkGetMacro(RemoveConstraintVertices, std::shared_ptr<std::unordered_set<size_t>>);
    imstkGetMacro(AddConstraintVertices, std::shared_ptr<std::unordered_set<size_t>>);

protected:
    void requestUpdate() override;

    void refinement(std::shared_ptr<SurfaceMesh> outputSurf,
                    std::map<int, bool>& cutVerts);

    void splitVerts(std::shared_ptr<SurfaceMesh> outputSurf,
                    std::map<int, bool>& cutVerts,
                    std::shared_ptr<Geometry> geometry);

    int pointOnGeometrySide(Vec3d pt, std::shared_ptr<Geometry> geometry);

    int pointOnAnalyticalSide(Vec3d pt, std::shared_ptr<AnalyticalGeometry> geometry);

    bool vertexOnBoundary(std::shared_ptr<VecDataArray<int, 3>> triangleIndices,
                          std::set<int>& triSet);

    bool pointProjectionInSurface(const Vec3d& pt, std::shared_ptr<SurfaceMesh> cutSurf);

    void generateAnalyticalCutData(std::shared_ptr<AnalyticalGeometry> geometry,
                                   std::shared_ptr<SurfaceMesh>        outputSurf);

    void generateSurfaceMeshCutData(std::shared_ptr<SurfaceMesh> cutSurf,
                                    std::shared_ptr<SurfaceMesh> outputSurf);

private:
    std::shared_ptr<std::vector<CutData>> m_CutData    = std::make_shared<std::vector<CutData>>();
    std::shared_ptr<std::map<int, int>>   m_CutVertMap = std::make_shared<std::map<int, int>>();
    std::shared_ptr<Geometry> m_CutGeometry = nullptr;
    std::shared_ptr<std::unordered_set<size_t>> m_RemoveConstraintVertices = nullptr;
    std::shared_ptr<std::unordered_set<size_t>> m_AddConstraintVertices    = nullptr;
    double m_Epsilon = 1;
};
}