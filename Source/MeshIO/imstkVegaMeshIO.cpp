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

#include "imstkVegaMeshIO.h"
#include "imstkMeshIO.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkVolumetricMesh.h"

#include "imstkLogger.h"

// Vega
#include "volumetricMeshLoader.h"
#include "volumetricMesh.h"

#include "tetMesh.h"

#include <array>

namespace imstk
{
std::shared_ptr<VolumetricMesh>
VegaMeshIO::read(const std::string& filePath, MeshFileType meshType)
{
    CHECK(meshType == MeshFileType::VEG) << "VegaMeshIO::read error: file type not supported";

    // Read Vega Mesh
    std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshIO::readVegaMesh(filePath);

    // Convert to Volumetric Mesh
    return VegaMeshIO::convertVegaMeshToVolumetricMesh(vegaMesh);
}

bool
VegaMeshIO::write(const std::shared_ptr<imstk::PointSet> imstkMesh, const std::string& filePath, const MeshFileType meshType)
{
    CHECK(meshType == MeshFileType::VEG) << "VegaMeshIO::write error: file type is not veg";

    // extract volumetric mesh
    const auto imstkVolMesh = std::dynamic_pointer_cast<imstk::VolumetricMesh>(imstkMesh);

    CHECK(imstkVolMesh != nullptr) << "VegaMeshIO::write error: imstk::Mesh is not a volumetric mesh";

    switch (imstkVolMesh->getType())
    {
    case Geometry::Type::TetrahedralMesh:
    case Geometry::Type::HexahedralMesh:
        auto vegaMesh = convertVolumetricMeshToVegaMesh(imstkVolMesh);

        CHECK(vegaMesh != nullptr) << "VegaMeshIO::write error: failed to convert volumetric mesh to vega mesh";

        const auto fileName     = const_cast<char*>(filePath.c_str());
        const int  write_status = vegaMesh->save(fileName);

        CHECK(write_status == 0) << "VegaMeshIO::write error: failed to write .veg file";

        return true;
        break;
    }

    LOG(WARNING) << "VegaMeshIO::write error: this element type not supported in vega";
    return false;
}

std::shared_ptr<vega::VolumetricMesh>
VegaMeshIO::readVegaMesh(const std::string& filePath)
{
    auto                                  fileName = const_cast<char*>(filePath.c_str());
    std::shared_ptr<vega::VolumetricMesh> vegaMesh(vega::VolumetricMeshLoader::load(fileName));
    return vegaMesh;
}

std::shared_ptr<imstk::VolumetricMesh>
VegaMeshIO::convertVegaMeshToVolumetricMesh(std::shared_ptr<vega::VolumetricMesh> vegaMesh)
{
    // Copy vertices
    StdVectorOfVec3d vertices;
    VegaMeshIO::copyVertices(vegaMesh, vertices);

    // Copy cells
    auto                                   cellType = vegaMesh->getElementType();
    std::shared_ptr<imstk::VolumetricMesh> mesh;
    if (cellType == vega::VolumetricMesh::TET)
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        VegaMeshIO::copyCells<4>(vegaMesh, cells);

        auto tetMesh = std::make_shared<TetrahedralMesh>();
        tetMesh->initialize(vertices, cells, false);
        mesh = tetMesh;
    }
    else if (cellType == vega::VolumetricMesh::CUBIC)
    {
        std::vector<HexahedralMesh::HexaArray> cells;
        VegaMeshIO::copyCells<8>(vegaMesh, cells);

        auto hexMesh = std::make_shared<HexahedralMesh>();
        hexMesh->initialize(vertices, cells, false);
        mesh = hexMesh;
    }
    else
    {
        vegaMesh.reset();
        LOG(WARNING) << "VegaMeshIO::read error: invalid cell type.";
        return nullptr;
    }
    return mesh;
}

void
VegaMeshIO::copyVertices(std::shared_ptr<vega::VolumetricMesh> vegaMesh,
                         StdVectorOfVec3d&                     vertices)
{
    for (int i = 0; i < vegaMesh->getNumVertices(); ++i)
    {
        auto pos = vegaMesh->getVertex(i);
        vertices.emplace_back(pos[0], pos[1], pos[2]);
    }
}

template<size_t dim>
void
VegaMeshIO::copyCells(std::shared_ptr<vega::VolumetricMesh> vegaMesh,
                      std::vector<std::array<size_t, dim>>& cells)
{
    std::array<size_t, dim> cell;
    for (size_t cellId = 0; cellId < vegaMesh->getNumElements(); ++cellId)
    {
        for (int i = 0; i < vegaMesh->getNumElementVertices(); ++i)
        {
            cell[i] = vegaMesh->getVertexIndex(int(cellId), i);
        }
        cells.emplace_back(cell);
    }
}

std::shared_ptr<vega::VolumetricMesh>
VegaMeshIO::convertVolumetricMeshToVegaMesh(const std::shared_ptr<imstk::VolumetricMesh> imstkVolMesh)
{
    // as of now, only works for TET elements
    if (imstkVolMesh->getType() == Geometry::Type::TetrahedralMesh)
    {
        // Using default material properties to append to the .veg file
        const double E       = 1E7;
        const double nu      = 0.4;
        const double density = 1000.0;

        auto imstkVolTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(imstkVolMesh);

        auto                vertexArray = imstkVolMesh->getVertexPositions();
        std::vector<double> vertices;
        for (const auto& node : vertexArray)
        {
            vertices.emplace_back(node(0));
            vertices.emplace_back(node(1));
            vertices.emplace_back(node(2));
        }

        auto             tetArray = imstkVolTetMesh->getTetrahedraVertices();
        std::vector<int> elements;
        for (const auto& tet : tetArray)
        {
            elements.emplace_back(int(tet[0]));
            elements.emplace_back(int(tet[1]));
            elements.emplace_back(int(tet[2]));
            elements.emplace_back(int(tet[3]));
        }

        std::shared_ptr<vega::TetMesh> vegaMesh = std::make_shared<vega::TetMesh>(int(imstkVolTetMesh->getNumVertices()), &vertices[0],
                int(imstkVolTetMesh->getNumTetrahedra()), &elements[0], E, nu, density);

        CHECK(vegaMesh != nullptr) << "VegaMeshIO::convertVolumetricMeshToVegaMesh error: Failed to create vega mesh";

        return vegaMesh;
    }
    else
    {
        LOG(WARNING) << "VegaMeshIO::convertVolumetricMeshToVegaMesh error: Geometry type not supported";
        return nullptr;
    }
}
} // imstk
