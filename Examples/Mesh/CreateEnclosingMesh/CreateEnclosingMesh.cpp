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

#include <cassert>
#include <memory>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "imstkAPIUtilities.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkVTKMeshIO.h"

using namespace imstk;

std::shared_ptr<SurfaceMesh> readMesh(const std::string& fname);

int
main()
{
    std::cout << "read a tet mesh and extract the surface mesh..." << std::endl;
    auto surfMesh = readMesh("../../../../install/data/asianDragon/asianDragon.veg");
    // auto surfMesh = readMesh("../../../../install/data/asianDragon/asianDragon.obj");

    std::cout << "create the enclosing mesh..." << std::endl;
    size_t nx = 80, ny = 40, nz = 40;
    auto tetMesh = TetrahedralMesh::createEnclosingMesh(*surfMesh, nx, ny, nz);
    tetMesh->writeVTK("tetMesh.vtk");
    surfMesh->writeVTK("surfMesh.vtk");
    // imstk::VTKMeshIO::write(surfMesh, "surfMesh.vtk", VTK);
}

std::shared_ptr<SurfaceMesh>
readMesh(const std::string& fname)
{

    // std::shared_ptr<TetrahedralMesh> mesh = std::dynamic_pointer_cast<TetrahedralMesh>(VTKMeshIO::read(fname, OBJ));
    // std::shared_ptr<TetrahedralMesh> mesh = std::dynamic_pointer_cast<TetrahedralMesh>(MeshIO::read(fname));
    
    std::ifstream     fin(fname);
    std::string       line;
    std::stringstream ss;

    size_t numVerts = 0;
    // skip comments and blank lines
    std::getline(fin, line);
    std::getline(fin, line);
    std::getline(fin, line);
    std::getline(fin, line);

    std::getline(fin, line);
    ss.str(line);
    ss >> numVerts;
    std::cout << "num of vertices = " << numVerts << std::endl;

    StdVectorOfVec3d coords(numVerts);
    size_t           vid;

    for (size_t i = 0; i < numVerts; ++i)
    {
        ss.clear();
        ss.str(std::string());
        std::getline(fin, line);
        ss.str(line);
        ss >> vid >> coords[i][0] >> coords[i][1] >> coords[i][2];
        assert(vid == i + 1);
    }

    size_t numTets;
    std::getline(fin, line);
    std::getline(fin, line);
    std::getline(fin, line);
    assert(line == "TET");

    std::getline(fin, line);
    ss.clear();
    ss.str(std::string());
    ss.str(line);
    ss >> numTets;
    std::cout << "num of tets = " << numTets << std::endl;

    std::vector<TetrahedralMesh::TetraArray> indices(numTets);

    size_t tid;

    for (size_t i = 0; i < numTets; ++i)
    {
        ss.clear();
        ss.str(std::string());
        std::getline(fin, line);
        ss.str(line);
        ss >> tid >> indices[i][0] >> indices[i][1] >> indices[i][2] >> indices[i][3];
        assert(tid == i + 1);
        --indices[i][0];
        --indices[i][1];
        --indices[i][2];
        --indices[i][3];
    }

    auto mesh = std::make_shared<TetrahedralMesh>();
    mesh->initialize(coords, indices);
    mesh->print();
    mesh->writeVTK("confMesh.vtk");

    auto surfMesh = std::make_shared<SurfaceMesh>();
    mesh->extractSurfaceMesh(surfMesh);
    surfMesh->print();

    return surfMesh;
}
