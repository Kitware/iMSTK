/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkHexahedralMesh.h"
#include "imstkLogger.h"
#include "imstkMacros.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVegaMeshIO.h"

DISABLE_WARNING_PUSH
    DISABLE_WARNING_HIDES_CLASS_MEMBER

#include <tetMesh.h>
#include <volumetricMeshLoader.h>

DISABLE_WARNING_POP

namespace imstk
{
std::shared_ptr<PointSet>
VegaMeshIO::read(const std::string& filePath, MeshFileType meshType)
{
    CHECK(meshType == MeshFileType::VEG) << "@VegaMeshIO::read error: input file type is not veg for input " << filePath;

    // Read Vega Mesh
    std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshIO::readVegaMesh(filePath);

    // Convert to Volumetric Mesh
    return VegaMeshIO::convertVegaMeshToVolumetricMesh(vegaMesh);
}

bool
VegaMeshIO::write(const std::shared_ptr<PointSet> imstkMesh, const std::string& filePath, const MeshFileType meshType)
{
    CHECK(meshType == MeshFileType::VEG) << "@VegaMeshIO::write error: file type is not veg for input " << filePath;

    // Extract volumetric mesh
    auto imstkVolMesh = std::dynamic_pointer_cast<PointSet>(imstkMesh);

    CHECK(imstkVolMesh != nullptr) << "@VegaMeshIO::write error: imstk::PointSet object supplied is not a imstk::VolumetricMesh type for input" << filePath;

    const std::string geometryType = imstkVolMesh->getTypeName();
    if (geometryType == TetrahedralMesh::getStaticTypeName() || geometryType == HexahedralMesh::getStaticTypeName())
    {
        auto vegaMesh = convertVolumetricMeshToVegaMesh(imstkVolMesh);

        CHECK(vegaMesh != nullptr) << "@VegaMeshIO::write error: failed to convert volumetric mesh to vega mesh for input" << filePath;

        const auto fileName     = const_cast<char*>(filePath.c_str());
        const int  write_status = vegaMesh->save(fileName);

        CHECK(write_status == 0) << "@VegaMeshIO::write error: failed (vega fail code " << write_status <<
            ". Check vega::VolumetricMesh::save for documentation)to write .veg file for input" << filePath;

        return true;
    }

    LOG(WARNING) << "Error: Unknown element type (only tetrahedral and hexahedral elements supported) for input " << filePath;
    return false;
}

std::shared_ptr<vega::VolumetricMesh>
VegaMeshIO::readVegaMesh(const std::string& filePath)
{
    auto                                  fileName = const_cast<char*>(filePath.c_str());
    std::shared_ptr<vega::VolumetricMesh> vegaMesh(vega::VolumetricMeshLoader::load(fileName));
    return vegaMesh;
}

std::shared_ptr<PointSet>
VegaMeshIO::convertVegaMeshToVolumetricMesh(std::shared_ptr<vega::VolumetricMesh> vegaMesh)
{
    // Copy vertices
    std::shared_ptr<VecDataArray<double, 3>> vertices = std::make_shared<VecDataArray<double, 3>>();
    VegaMeshIO::copyVertices(vegaMesh, *vertices);

    // Copy cells
    auto                      cellType = vegaMesh->getElementType();
    std::shared_ptr<PointSet> mesh;
    if (cellType == vega::VolumetricMesh::TET)
    {
        std::shared_ptr<VecDataArray<int, 4>> cellsPtr = std::make_shared<VecDataArray<int, 4>>();
        VecDataArray<int, 4>&                 cells    = *cellsPtr;
        VegaMeshIO::copyCells<4>(vegaMesh, cells);

        auto tetMesh = std::make_shared<TetrahedralMesh>();
        tetMesh->initialize(vertices, cellsPtr);
        mesh = tetMesh;
    }
    else if (cellType == vega::VolumetricMesh::CUBIC)
    {
        std::shared_ptr<VecDataArray<int, 8>> cellsPtr = std::make_shared<VecDataArray<int, 8>>();
        VecDataArray<int, 8>&                 cells    = *cellsPtr;
        VegaMeshIO::copyCells<8>(vegaMesh, cells);

        auto hexMesh = std::make_shared<HexahedralMesh>();
        hexMesh->initialize(vertices, cellsPtr);
        mesh = hexMesh;
    }
    else
    {
        vegaMesh.reset();
        LOG(WARNING) << "Error: invalid cell type";
        return nullptr;
    }
    return mesh;
}

void
VegaMeshIO::copyVertices(std::shared_ptr<vega::VolumetricMesh> vegaMesh,
                         VecDataArray<double, 3>& vertices)
{
    for (int i = 0; i < vegaMesh->getNumVertices(); ++i)
    {
        auto pos = vegaMesh->getVertex(i);
        vertices.push_back(Vec3d(pos[0], pos[1], pos[2]));
    }
}

template<size_t dim>
void
VegaMeshIO::copyCells(std::shared_ptr<vega::VolumetricMesh> vegaMesh,
                      VecDataArray<int, dim>& cells)
{
    typename VecDataArray<int, dim>::ValueType cell;
    for (int cellId = 0; cellId < vegaMesh->getNumElements(); ++cellId)
    {
        for (int i = 0; i < vegaMesh->getNumElementVertices(); ++i)
        {
            cell[i] = vegaMesh->getVertexIndex(static_cast<int>(cellId), i);
        }
        cells.push_back(cell);
    }
}

std::shared_ptr<vega::VolumetricMesh>
VegaMeshIO::convertVolumetricMeshToVegaMesh(const std::shared_ptr<PointSet> imstkVolMesh)
{
    // as of now, only works for TET elements
    if (imstkVolMesh->getTypeName() == TetrahedralMesh::getStaticTypeName())
    {
        auto imstkVolTetMesh = std::dynamic_pointer_cast<imstk::TetrahedralMesh>(imstkVolMesh);

        auto                vertexArray = *imstkVolMesh->getVertexPositions();
        std::vector<double> vertices;
        for (const auto& node : vertexArray)
        {
            vertices.emplace_back(node(0));
            vertices.emplace_back(node(1));
            vertices.emplace_back(node(2));
        }

        VecDataArray<int, 4>& tetArray = *imstkVolTetMesh->getCells();
        std::vector<int>      elements;
        for (const Vec4i& tet : tetArray)
        {
            elements.emplace_back(int(tet[0]));
            elements.emplace_back(int(tet[1]));
            elements.emplace_back(int(tet[2]));
            elements.emplace_back(int(tet[3]));
        }

        // Using default material properties to append to the .veg file
        auto vegaMesh = std::make_shared<vega::TetMesh>(int(imstkVolTetMesh->getNumVertices()),
            &vertices[0],
            int(imstkVolTetMesh->getNumCells()),
            &elements[0],
            1E7, /* Young's modulus */
            0.4, /* Poisson's ratio */
            1000.0 /* density */);

        CHECK(vegaMesh != nullptr) << "@VegaMeshIO::convertVolumetricMeshToVegaMesh error: Failed to create vega mesh. "
                                   << "Check vega::TetMesh::TetMesh() for clues";

        return vegaMesh;
    }
    else
    {
        LOG(WARNING) << "Error: Geometry type other than tetrahedral mesh not supported";
        return nullptr;
    }
}
} // namespace imstk