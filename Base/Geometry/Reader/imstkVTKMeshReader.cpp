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

#include "imstkVTKMeshReader.h"

#include "vtkSmartPointer.h"
#include "vtkGenericDataObjectReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkPLYReader.h"
#include "vtkOBJReader.h"
#include "vtkSTLReader.h"
#include "vtkFloatArray.h"

#include "g3log/g3log.hpp"

namespace imstk {
std::shared_ptr<Mesh>
VTKMeshReader::read(const std::string& filePath, MeshReader::FileType meshType)
{
    switch (meshType)
    {
    case MeshReader::FileType::VTK :
    {
        return VTKMeshReader::readVtkGenericFormatData<vtkGenericDataObjectReader>(filePath);
        break;
    }
    case MeshReader::FileType::VTU :
    {
        return VTKMeshReader::readVtkUnstructuredGrid<vtkXMLUnstructuredGridReader>(filePath);
        break;
    }
    case MeshReader::FileType::VTP :
    {
        return VTKMeshReader::readVtkPolyData<vtkXMLPolyDataReader>(filePath);
        break;
    }
    case MeshReader::FileType::STL :
    {
        return VTKMeshReader::readVtkPolyData<vtkSTLReader>(filePath);
        break;
    }
    case MeshReader::FileType::PLY :
    {
        return VTKMeshReader::readVtkPolyData<vtkPLYReader>(filePath);
        break;
    }
    case MeshReader::FileType::OBJ :
    {
        return VTKMeshReader::readVtkPolyData<vtkOBJReader>(filePath);
        break;
    }
    default :
    {
        LOG(WARNING) << "VTKMeshReader::read error: file type not supported";
        break;
    }
    }
}

template<typename ReaderType>
std::shared_ptr<Mesh>
VTKMeshReader::readVtkGenericFormatData(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    if (vtkPolyData* vtkMesh = reader->GetPolyDataOutput())
    {
        return VTKMeshReader::convertVtkPolyDataToSurfaceMesh(vtkMesh);
    }
    else if (vtkUnstructuredGrid* vtkMesh = reader->GetUnstructuredGridOutput())
    {
        return VTKMeshReader::convertVtkUnstructuredGridToVolumetricMesh(vtkMesh);
    }
    else
    {
        LOG(WARNING) << "VTKMeshReader::readVtkGenericFormatData error: could not read with VTK reader.";
        return nullptr;
    }
}

template<typename ReaderType>
std::shared_ptr<SurfaceMesh>
VTKMeshReader::readVtkPolyData(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    vtkPolyData* vtkMesh = reader->GetOutput();
    return VTKMeshReader::convertVtkPolyDataToSurfaceMesh(vtkMesh);
}

template<typename ReaderType>
std::shared_ptr<VolumetricMesh>
VTKMeshReader::readVtkUnstructuredGrid(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    vtkUnstructuredGrid* vtkMesh = reader->GetOutput();
    return VTKMeshReader::convertVtkUnstructuredGridToVolumetricMesh(vtkMesh);
}

std::shared_ptr<SurfaceMesh>
VTKMeshReader::convertVtkPolyDataToSurfaceMesh(vtkPolyData* vtkMesh)
{
    if(!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshReader::convertVtkPolyDataToSurfaceMesh error: could not read with VTK reader.";
        return nullptr;
    }

    std::vector<Vec3d> vertices;
    VTKMeshReader::copyVertices(vtkMesh->GetPoints(), vertices);

    std::vector<SurfaceMesh::TriangleArray> triangles;
    VTKMeshReader::copyCells<3>(vtkMesh->GetPolys(), triangles);

    std::vector<Vec2f> textCoords;
    VTKMeshReader::copyTextureCoordinates(vtkMesh->GetPointData(), textCoords);

    auto mesh = std::make_shared<SurfaceMesh>();
    mesh->initialize(vertices, triangles, textCoords, true);
    return mesh;
}

std::shared_ptr<VolumetricMesh>
VTKMeshReader::convertVtkUnstructuredGridToVolumetricMesh(vtkUnstructuredGrid* vtkMesh)
{
    if(!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshReader::convertVtkUnstructuredGridToVolumetricMesh error: could not read with VTK reader.";
        return nullptr;
    }

    std::vector<Vec3d> vertices;
    VTKMeshReader::copyVertices(vtkMesh->GetPoints(), vertices);

    vtkIdType cellType;
    vtkMesh->GetCellType(cellType);
    if( cellType == VTK_TETRA )
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        VTKMeshReader::copyCells<4>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_shared<TetrahedralMesh>();
        mesh->initialize(vertices, cells, true);
        return mesh;
    }
    else if( cellType == VTK_HEXAHEDRON )
    {
        const size_t dim = 8;
        std::vector<HexahedralMesh::HexaArray> cells;
        VTKMeshReader::copyCells<8>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_shared<HexahedralMesh>();
        mesh->initialize(vertices, cells, true);
        return mesh;
    }
    else
    {
        LOG(WARNING) << "VTKMeshReader::convertVtkUnstructuredGridToVolumetricMesh error: No support for vtkCellType="
                     << cellType << ".";
        return nullptr;
    }
}

void
VTKMeshReader::copyVertices(vtkPoints* points, std::vector<Vec3d>& vertices)
{
    if(!points)
    {
        LOG(WARNING) << "VTKMeshReader::copyVertices error: No points found.";
        return;
    }

    for(vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
        double pos[3];
        points->GetPoint(i, pos);
        vertices.emplace_back(pos[0], pos[1], pos[2]);
    }
}

template<size_t dim>
void
VTKMeshReader::copyCells(vtkCellArray* vtkCells, std::vector<std::array<size_t,dim>>& cells)
{
    if(!vtkCells)
    {
        LOG(WARNING) << "VTKMeshReader::copyCells error: No cells found.";
        return;
    }

    vtkCells->InitTraversal();
    auto vtkCell = vtkSmartPointer<vtkIdList>::New();
    std::array<size_t, dim> cell;
    while(vtkCells->GetNextCell(vtkCell))
    {
        for(size_t i = 0; i < dim; ++i)
        {
            cell[i] = vtkCell->GetId(i);
        }
        cells.emplace_back(cell);
    }
}

void
VTKMeshReader::copyTextureCoordinates(vtkPointData* pointData, std::vector<Vec2f>& textCoords)
{
    if(!pointData)
    {
        return;
    }

    auto tcoords = vtkFloatArray::SafeDownCast(pointData->GetTCoords());
    if(!tcoords)
    {
        return;
    }

    for(vtkIdType i = 0; i < tcoords->GetNumberOfTuples(); ++i)
    {
        float uv[2];
        tcoords->GetTupleValue(i, uv);
        textCoords.emplace_back(uv[0], uv[1]);
    }
}

}
