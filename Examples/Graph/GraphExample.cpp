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

#include "imstkAPIUtilities.h"
#include "imstkGraph.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

using namespace imstk;

///
/// \brief This example demonstrates the imstk graph usage
/// Usage: Example-Graph.exe [method=greedy/welsh-powell]
/// (if no method was specified, WelshPowell method will be used)
///
int
main(int argc, char** argv)
{
    Logger::startLogger();

    // Using WelshPowell method by default
    Graph::ColoringMethod method = Graph::ColoringMethod::WelshPowell;

    if (argc > 1)
    {
        auto param = std::string(argv[1]);
        if (param.find("method") == 0
            && param.find_first_of("=") != std::string::npos)
        {
            if (param.substr(param.find_first_of("=") + 1) == "greedy")
            {
                method = Graph::ColoringMethod::Greedy;
            }
            else if (param.substr(param.find_first_of("=") + 1) == "welsh-powell")
            {
                method = Graph::ColoringMethod::WelshPowell;
            }
        }
    }

    std::cout << (method == Graph::ColoringMethod::Greedy ?
                  "Graph coloring method: Greedy" :
                  "Graph coloring method: WelshPowell") << std::endl << std::endl;

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

    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    if (!tetMesh)
    {
        LOG(INFO) << "Could not read mesh from file.";
        return 1;
    }
    else
    {
        auto colorsGVMesh = apiutils::getMeshGraph(tetMesh)->doColoring(method, true);

        imstkNew<SurfaceMesh> surfMesh;
        tetMesh->extractSurfaceMesh(surfMesh, true);
        auto colorsGSMesh = apiutils::getMeshGraph(surfMesh.get())->doColoring(method, true);
    }

    LOG(INFO) << "Press any key to exit!";
    getchar();

    return 0;
}
