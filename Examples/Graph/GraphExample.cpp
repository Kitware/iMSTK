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

#include "imstkGraph.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"

using namespace imstk;

///
/// \brief This example demonstrates the imstk graph usage
///
int main(int argc, char** argv)
{
    Graph::ColoringMethod method = Graph::ColoringMethod::WelshPowell;
    if (argc > 1)
    {
        auto param = std::string(argv[1]);
        if (param.find("method") == 0 &&
            param.find_first_of("=") != std::string::npos)
        {
            if (param.substr(param.find_first_of("=") + 1) == "greedy")
            {
                method = Graph::ColoringMethod::Greedy;
            }
            // else: no need to set
        }
    }

    Graph g1(5);
    g1.addEdge(0, 1);
    g1.addEdge(0, 2);
    g1.addEdge(1, 2);
    g1.addEdge(1, 3);
    g1.addEdge(2, 3);
    g1.addEdge(3, 4);

    g1.print();
    auto colorsG1 = g1.doColoring(method, true);

    Graph g2(5);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 2);
    g2.addEdge(1, 4);
    g2.addEdge(2, 4);
    g2.addEdge(4, 3);

    g2.print();
    auto colorsG2 = g2.doColoring(method, true);

    auto tetMesh = MeshIO::read(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        std::cout << "Could not read mesh from file." << std::endl;
        return 1;
    }
    else
    {
        auto volMesh = std::dynamic_pointer_cast<TetrahedralMesh>(tetMesh);
        if (!volMesh)
        {
            LOG(WARNING) << "Dynamic pointer cast from PointSet to TetrahedralMesh failed!";
            return 1;
        }
        auto colorsGVMesh = volMesh->getMeshGraph()->doColoring(method, true);

        auto surfMesh = std::make_shared<SurfaceMesh>();
        volMesh->extractSurfaceMesh(surfMesh, true);
        auto colorsGSMesh = surfMesh->getMeshGraph()->doColoring(method, true);
    }

    std::cout << "Press any key to exit!" << std::endl;
    getchar();

    return 0;
}
