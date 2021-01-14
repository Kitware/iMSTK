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

namespace imstk
{
class Plane;

// vertex on the plane (0), positive side (+1), negative side (-1)
// pt0 and pt1 follows the triangle's indexing order when tri is presented
enum class CutType
{
    NONE = 0,
    /* triangle is not cut through
    *     pt0 (-+1)
    *         /  \
    *        /    \
    *       / tri  \
    * pt1 (+-1)------(?)
    */
    EDGE,
    /*
    *      (-+1)
    *       /  \
    *      /    \
    *     / tri  \
    *  (-+1)------(0) pt0
    */
    VERT,
    /*
    *        (+-1) pt1
    *        /  \
    *       /    \
    *      / tri  \
    *  (-+1)------(+-1) pt0
    */
    EDGE_EDGE,
    /*
    *        pt0 (+-1)
    *            /  \
    *           /    \
    *          / tri  \
    *  pt1 (-+1)------(0)
    */
    EDGE_VERT,
    /*
    *    pt0 (0)-----(+-1)
    *       /  \      /
    *      /    \    /
    *     /      \  /
    *  (-+1)------(0) pt1
    */
    VERT_VERT
};

struct CutData
{
    public:
        Vec3d cutCoords[2];
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

    imstkGetMacro(CutData, std::shared_ptr<std::vector<CutData>>);
    imstkSetMacro(CutData, std::shared_ptr<std::vector<CutData>>);
    imstkGetMacro(Plane, std::shared_ptr<Plane>);
    imstkSetMacro(Plane, std::shared_ptr<Plane>);
    imstkGetMacro(Epsilon, double);
    imstkSetMacro(Epsilon, double);

protected:
    void requestUpdate() override;
    void refinement(std::shared_ptr<SurfaceMesh> outputSurf,
                    std::map<int, bool>& cutVerts);

    void splitVerts(std::shared_ptr<SurfaceMesh> outputSurf,
                    std::map<int, bool>& cutVerts);
    int pointOnPlaneSide(Vec3d pt);
    bool vertexOnBoundary(std::shared_ptr<VecDataArray<int, 3>> triangleIndices,
                          std::set<int>& triSet);

    void generateCutData(std::shared_ptr<Plane> plane, std::shared_ptr<SurfaceMesh> inputSurf);
// void generateCutData(triangle);

private:
    std::shared_ptr<std::vector<CutData>> m_CutData = std::make_shared<std::vector<CutData>>();
    std::shared_ptr<Plane> m_Plane = std::make_shared<Plane>();
    double m_Epsilon = 1;
};
}