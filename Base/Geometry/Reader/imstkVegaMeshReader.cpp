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

#include "imstkVegaMeshReader.h"

#include "imstkMeshReader.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"

// Vega
#include "volumetricMeshLoader.h"
#include "volumetricMesh.h"

#include "g3log/g3log.hpp"

namespace imstk{
std::shared_ptr<VolumetricMesh>
VegaMeshReader::read(const std::string& filePath, MeshFileType meshType)
{
    if (meshType != MeshFileType::VEG)
    {
        LOG(WARNING) << "VegaMeshReader::read error: file type not supported";
        return nullptr;
    }

    // Vega VolumetricMesh (not imstk::)
    auto fileName = const_cast<char*>(filePath.c_str());
    std::shared_ptr<vega::VolumetricMesh> vegaMesh(vega::VolumetricMeshLoader::load(fileName));

    // Copy vertices
    std::vector<Vec3d> vertices;
    for(size_t i = 0; i < vegaMesh->getNumVertices(); ++i)
    {
        auto pos = *vegaMesh->getVertex(i);
        vertices.emplace_back(pos[0], pos[1], pos[2]);
    }

    // Check
    auto cellType = vegaMesh->getElementType();
    if(cellType == vega::VolumetricMesh::TET)
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        TetrahedralMesh::TetraArray cell;
        for(size_t cellId = 0; cellId < vegaMesh->getNumElements(); ++cellId)
        {
            for (size_t i = 0; i < vegaMesh->getNumElementVertices(); ++i)
            {
                cell[i] = vegaMesh->getVertexIndex(cellId,i);
            }
            cells.emplace_back(cell);
        }

        auto mesh = std::make_shared<TetrahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else if(cellType == vega::VolumetricMesh::CUBIC)
    {
        std::vector<HexahedralMesh::HexaArray> cells;
        HexahedralMesh::HexaArray cell;
        for(size_t cellId = 0; cellId < vegaMesh->getNumElements(); ++cellId)
        {
            for (size_t i = 0; i < vegaMesh->getNumElementVertices(); ++i)
            {
                cell[i] = vegaMesh->getVertexIndex(cellId,i);
            }
            cells.emplace_back(cell);
        }

        auto mesh = std::make_shared<HexahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else
    {
        vegaMesh.reset();
        LOG(WARNING) << "VegaMeshReader::read error: invalid cell type.";
        return nullptr;
    }
}
}
