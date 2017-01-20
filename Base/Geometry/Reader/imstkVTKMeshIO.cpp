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

#include "imstkVTKMeshIO.h"

#include "vtkSmartPointer.h"
#include "vtkGenericDataObjectReader.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkPLYReader.h"
#include "vtkOBJReader.h"
#include "vtkSTLReader.h"
#include "vtkFloatArray.h"
#include "vtkTriangleFilter.h"

#include "g3log/g3log.hpp"

namespace imstk
{

std::shared_ptr<Mesh>
VTKMeshIO::read(const std::string& filePath, MeshFileType meshType)
{
    switch (meshType)
    {
    case MeshFileType::VTK :
    {
        return VTKMeshIO::readVtkGenericFormatData<vtkGenericDataObjectReader>(filePath);
        break;
    }
    case MeshFileType::VTU :
    {
        return VTKMeshIO::readVtkUnstructuredGrid<vtkXMLUnstructuredGridReader>(filePath);
        break;
    }
    case MeshFileType::VTP :
    {
        return VTKMeshIO::readVtkPolyData<vtkXMLPolyDataReader>(filePath);
        break;
    }
    case MeshFileType::STL :
    {
        return VTKMeshIO::readVtkPolyData<vtkSTLReader>(filePath);
        break;
    }
    case MeshFileType::PLY :
    {
        return VTKMeshIO::readVtkPolyData<vtkPLYReader>(filePath);
        break;
    }
    case MeshFileType::OBJ :
    {
        return VTKMeshIO::readVtkPolyData<vtkOBJReader>(filePath);
        break;
    }
    default :
    {
        LOG(WARNING) << "VTKMeshIO::read error: file type not supported";
        return nullptr;
        break;
    }
    }
}

template<typename ReaderType>
std::shared_ptr<Mesh>
VTKMeshIO::readVtkGenericFormatData(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    if (vtkPolyData* vtkMesh = reader->GetPolyDataOutput())
    {
        return VTKMeshIO::convertVtkPolyDataToSurfaceMesh(vtkMesh);
    }
    else if (vtkUnstructuredGrid* vtkMesh = reader->GetUnstructuredGridOutput())
    {
        return VTKMeshIO::convertVtkUnstructuredGridToVolumetricMesh(vtkMesh);
    }
    else
    {
        LOG(WARNING) << "VTKMeshIO::readVtkGenericFormatData error: could not read with VTK reader.";
        return nullptr;
    }
}

template<typename ReaderType>
std::shared_ptr<SurfaceMesh>
VTKMeshIO::readVtkPolyData(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    auto triFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triFilter->SetInputData(reader->GetOutput());
    triFilter->Update();

    vtkPolyData* vtkMesh = triFilter->GetOutput();
    return VTKMeshIO::convertVtkPolyDataToSurfaceMesh(vtkMesh);
}

template<typename ReaderType>
std::shared_ptr<VolumetricMesh>
VTKMeshIO::readVtkUnstructuredGrid(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    vtkUnstructuredGrid* vtkMesh = reader->GetOutput();
    return VTKMeshIO::convertVtkUnstructuredGridToVolumetricMesh(vtkMesh);
}

std::shared_ptr<SurfaceMesh>
VTKMeshIO::convertVtkPolyDataToSurfaceMesh(vtkPolyData* vtkMesh)
{
    if(!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshIO::convertVtkPolyDataToSurfaceMesh error: could not read with VTK reader.";
        return nullptr;
    }

    StdVectorOfVec3d vertices;
    VTKMeshIO::copyVertices(vtkMesh->GetPoints(), vertices);

    std::vector<SurfaceMesh::TriangleArray> triangles;
    VTKMeshIO::copyCells<3>(vtkMesh->GetPolys(), triangles);

    auto mesh = std::make_shared<SurfaceMesh>();
    mesh->initialize(vertices, triangles, true);

    // Point Data
    std::map<std::string, StdVectorOfVectorf> dataMap;
    VTKMeshIO::copyPointData(vtkMesh->GetPointData(), dataMap);
    if (!dataMap.empty())
    {
      mesh->setPointDataMap(dataMap);
    }

    // Active Texture
    if (auto pointData = vtkMesh->GetPointData())
    if (auto tcoords = pointData->GetTCoords())
    {
        mesh->setDefaultTCoords(tcoords->GetName());
    }

    return mesh;
}

std::shared_ptr<VolumetricMesh>
VTKMeshIO::convertVtkUnstructuredGridToVolumetricMesh(vtkUnstructuredGrid* vtkMesh)
{
    if(!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshIO::convertVtkUnstructuredGridToVolumetricMesh error: could not read with VTK reader.";
        return nullptr;
    }

    StdVectorOfVec3d vertices;
    VTKMeshIO::copyVertices(vtkMesh->GetPoints(), vertices);

    int cellType = vtkMesh->GetCellType(vtkMesh->GetNumberOfCells()-1);
    if( cellType == VTK_TETRA )
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        VTKMeshIO::copyCells<4>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_shared<TetrahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else if( cellType == VTK_HEXAHEDRON )
    {
        std::vector<HexahedralMesh::HexaArray> cells;
        VTKMeshIO::copyCells<8>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_shared<HexahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else
    {
        LOG(WARNING) << "VTKMeshIO::convertVtkUnstructuredGridToVolumetricMesh error: No support for vtkCellType="
                     << cellType << ".";
        return nullptr;
    }
}

void
VTKMeshIO::copyVertices(vtkPoints* points, StdVectorOfVec3d& vertices)
{
    if(!points)
    {
        LOG(WARNING) << "VTKMeshIO::copyVertices error: No points found.";
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
VTKMeshIO::copyCells(vtkCellArray* vtkCells, std::vector<std::array<size_t,dim>>& cells)
{
    if(!vtkCells)
    {
        LOG(WARNING) << "VTKMeshIO::copyCells error: No cells found.";
        return;
    }

    vtkCells->InitTraversal();
    auto vtkCell = vtkSmartPointer<vtkIdList>::New();
    std::array<size_t, dim> cell;
    while(vtkCells->GetNextCell(vtkCell))
    {
        if (vtkCell->GetNumberOfIds() != dim)
        {
            continue;
        }
        for(size_t i = 0; i < dim; ++i)
        {
            cell[i] = vtkCell->GetId(i);
        }
        cells.emplace_back(cell);
    }
}

void
VTKMeshIO::copyPointData(vtkPointData* pointData, std::map<std::string, StdVectorOfVectorf>& dataMap)
{
    if(!pointData)
    {
        return;
    }

    for (unsigned int i = 0; i < pointData->GetNumberOfArrays(); ++i)
    {
        vtkDataArray* array = pointData->GetArray(i);
        std::string name = array->GetName();
        int nbrOfComp = array->GetNumberOfComponents();
        int nbrOfTuples = array->GetNumberOfTuples();
        StdVectorOfVectorf data;
        for(unsigned int j = 0; j < nbrOfTuples; ++j)
        {
            double* tupleData = new double [nbrOfComp];
            array->GetTuple(j, tupleData);
            Vectorf tuple(nbrOfComp);
            for (unsigned int k = 0; k < nbrOfComp; k++)
              {
              tuple[k] = tupleData[k];
              }
            data.push_back(tuple);
        }
        dataMap[name] = data;
    }
}

} // imstk
