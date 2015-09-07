/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */


#include "Core/Factory.h"
#include "IO/IOMeshDelegate.h"
#include "Mesh//VegaVolumetricMesh.h"

// Vega includes
#include "volumetricMesh.h"
#include "cubicMesh.h"
#include "tetMesh.h"

///
/// \brief Class delegate to read the Vega (.veg) volumetric file format. This
/// reader also computes the surface of the mesh and stores it as a \SurfaceMesh.
/// This mesh is then attached to the volumetric mesh and updated as the
/// underline volume mesh deforms and moves.
class IOMeshVegaDelegate : public IOMeshDelegate
{
public:
    void read();
    void write(){}
};

//----------------------------------------------------------------------------------------
void IOMeshVegaDelegate::read()
{
    // const cast to silence warnings later on
    auto name = const_cast<char*>(this->meshIO->getFileName().c_str());

    std::shared_ptr<VolumetricMesh> mesh;

    auto elementType = VolumetricMesh::getElementType(name);

    switch(elementType)
    {
        case VolumetricMesh::TET:
        {
            mesh = std::make_shared<TetMesh>(name, true);
            this->meshProps |= MeshType::Tetra;
            break;
        }

        case VolumetricMesh::CUBIC:
        {
            mesh = std::make_shared<CubicMesh>(name, true);
            this->meshProps |= MeshType::Hexa;
            break;
        }

        default:
        {
            mesh.reset();
            std::cerr << "Unknown element type." << std::endl;
        }
    }

    auto volumetricMesh = std::make_shared<VegaVolumetricMesh>(true);
    volumetricMesh->setVegaMesh(mesh);

    // Use vega to compute the surface triangles
    ObjMesh *vegaObjMesh = GenerateSurfaceMesh::ComputeMesh(mesh.get());
    const ObjMesh::Group *vegaObjMeshGroup = vegaObjMesh->getGroupHandle(0);

    // Copy triangles from vega structure...
    std::vector<std::array<size_t, 3>> localTriangleArray(vegaObjMesh->getNumFaces());

    for(int i = 0, end = vegaObjMeshGroup->getNumFaces(); i < end; ++i)
    {
        localTriangleArray[i][0] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(0)->getPositionIndex();
        localTriangleArray[i][1] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(1)->getPositionIndex();
        localTriangleArray[i][2] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(2)->getPositionIndex();
    }

    delete vegaObjMesh;

    // copy vertices
    std::vector<core::Vec3d> vertices;

    for(size_t i = 0, end = mesh->getNumVertices(); i < end; ++i)
    {
        const Vec3d &v = *mesh->getVertex(i);
        vertices.emplace_back(v[0],
                              v[1],
                              v[2]);
    }

    std::vector<core::Vec3d> surfaceVertices;
    std::unordered_map<size_t, size_t> uniqueVertexArray;
    this->reorderSurfaceTopology(
        vertices,
        surfaceVertices,
        localTriangleArray,
        uniqueVertexArray);

    auto meshToAttach = std::make_shared<SurfaceMesh>();
    meshToAttach->setVertices(surfaceVertices);
    meshToAttach->setTriangles(localTriangleArray);
    meshToAttach->updateOriginalVertsWithCurrent();

    volumetricMesh->setVertexMap(uniqueVertexArray);
    volumetricMesh->attachSurfaceMesh(meshToAttach);
    this->meshIO->setMesh(volumetricMesh);
}

RegisterFactoryClass(IOMeshDelegate,IOMeshVegaDelegate,IOMesh::ReaderGroup::Vega);
