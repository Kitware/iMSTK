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


#include "imstkSimulationManager.h"
#include "imstkTetrahedralMesh.h"

using namespace imstk;

void extractSurfaceMesh()
{
    auto sdk = std::make_shared<SimulationManager>();

    // a. Construct a sample tetrahedral mesh

    // a.1 add vertex positions
    auto tetMesh = std::make_shared<TetrahedralMesh>();
    StdVectorOfVec3d vertList;
    vertList.push_back(Vec3d(0, 0, 0));
    vertList.push_back(Vec3d(1.0, 0, 0));
    vertList.push_back(Vec3d(0, 1.0, 0));
    vertList.push_back(Vec3d(0, 0, 1.0));
    vertList.push_back(Vec3d(1.0, 1.0, 1.0));
    tetMesh->setInitialVertexPositions(vertList);
    tetMesh->setVertexPositions(vertList);

    // a.2 add connectivity
    std::vector<TetrahedralMesh::TetraArray> tetConnectivity;
    TetrahedralMesh::TetraArray tet1 = { 0, 1, 2, 3 };
    TetrahedralMesh::TetraArray tet2 = { 1, 2, 3, 4 };
    tetConnectivity.push_back(tet1);
    tetConnectivity.push_back(tet2);
    tetMesh->setTetrahedraVertices(tetConnectivity);

    // b. Print tetrahedral mesh
    tetMesh->print();

    // c. Extract the surface mesh
    auto surfMesh = std::make_shared<SurfaceMesh>();
    if (tetMesh->extractSurfaceMesh(surfMesh))
    {
        // c.1. Print the resulting mesh
        surfMesh->print();
    }
    else
    {
        LOG(WARNING) << "Surface mesh was not extracted!";
    }

    getchar();
}

int main()
{
    extractSurfaceMesh();

    return 0;
}
