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
#include "imstkGeometryUtilities.h"
#include <chrono> 
using namespace std::chrono; 

using namespace imstk;

bool verifyColoring(const Graph& graph, const std::vector<unsigned short>& colors)
{

    std::unordered_set<size_t> edges;
    for (size_t i=0; i<graph.size(); ++i)
    {
        unsigned short color_i = colors[i];
        graph.getEdges(i, edges);
        for (auto j : edges)
        {
            if (color_i == colors[j])
            {
                std::cout << "edge(" << i << "," << j << "): same color!" << std::endl;
                return false;
            }
        }
    }
    return true;
}


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
    auto colorsG1 = g1.doColoring(method, false);
    verifyColoring(g1, colorsG1.first);

    Graph g2(5);
    g2.addEdge(0, 1);
    g2.addEdge(0, 2);
    g2.addEdge(1, 2);
    g2.addEdge(1, 4);
    g2.addEdge(2, 4);
    g2.addEdge(4, 3);

    g2.print();
    auto colorsG2 = g2.doColoring(method, false);
    verifyColoring(g2, colorsG2.first);

    // auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    const int nx      = 160;
    const int ny      = 160;
    const int nz      = 160;
    auto      tetMesh = GeometryUtils::createUniformMesh(Vec3d{ -2.5, -2.5, -2.5 }, Vec3d{ 2.5, 2.5, 2.5 }, nx, ny, nz);
    if (!tetMesh)
    {
        std::cout << "Could not read mesh from file." << std::endl;
        LOG(INFO) << "Could not read mesh from file.";
        return 1;
    }
    else
    {
        // auto colorsGVMesh = apiutils::getMeshGraph(tetMesh)->doColoring(method, true);
        {
            auto graph = apiutils::getMeshGraph(tetMesh);
            auto t_start = high_resolution_clock::now(); 
            auto colorsGVMesh = graph->doColoring(method, false);
            verifyColoring(*graph, colorsGVMesh.first);
            std::cout << "number of colors = " << colorsGVMesh.second << std::endl;
            auto t_end = high_resolution_clock::now();
            auto duration = duration_cast<microseconds>(t_end - t_start); 

            std::cout << "runtime = " << duration.count() << " microseconds" << endl; 
        }

        // imstkNew<SurfaceMesh> surfMesh;
        // tetMesh->extractSurfaceMesh(surfMesh, true);
        // auto colorsGSMesh = apiutils::getMeshGraph(surfMesh.get())->doColoring(method, true);
    }

    LOG(INFO) << "Press any key to exit!";
    getchar();

    return 0;
}


