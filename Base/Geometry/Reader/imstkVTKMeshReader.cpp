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
#include "vtkXMLGenericDataObjectReader.h"
#include "vtkPLYReader.h"
#include "vtkOBJReader.h"
#include "vtkSTLReader.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"

#include "g3log/g3log.hpp"

namespace imstk {
std::shared_ptr<Mesh>
VTKMeshReader::read(const std::string& filePath, MeshReader::FileType meshType)
{
    std::vector<Vec3d> vertices;
    std::vector<Vec2f> textCoords;
    std::vector<SurfaceMesh::TriangleArray> triangles;
    std::vector<TetrahedralMesh::TetraArray> tetrahedra;
    std::vector<HexahedralMesh::HexaArray> hexahedra;

    // Populate vectors and arrays
    switch (meshType)
    {
    case MeshReader::FileType::VTK :
    {
        VTKMeshReader::readAsGenericFormatData<vtkGenericDataObjectReader>
                (filePath, vertices, triangles, tetrahedra, hexahedra);
        break;
    }
    case MeshReader::FileType::VTU :
    case MeshReader::FileType::VTP :
    {
        VTKMeshReader::readAsGenericFormatData<vtkXMLGenericDataObjectReader>
                (filePath, vertices, triangles, tetrahedra, hexahedra);
        break;
    }
    case MeshReader::FileType::STL :
    {
        VTKMeshReader::readAsAbstractPolyData<vtkSTLReader>(filePath, vertices, triangles);
        break;
    }
    case MeshReader::FileType::PLY :
    {
        VTKMeshReader::readAsAbstractPolyData<vtkSTLReader>(filePath, vertices, triangles);
        break;
    }
    case MeshReader::FileType::OBJ :
    {
        VTKMeshReader::readAsAbstractPolyData<vtkSTLReader>(filePath, vertices, triangles);
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
void
VTKMeshReader::readAsGenericFormatData(const std::string& filePath,
                                       std::vector<Vec3d>& vertices,
                                       std::vector<SurfaceMesh::TriangleArray>& triangles,
                                       std::vector<TetrahedralMesh::TetraArray>& tetrahedra,
                                       std::vector<HexahedralMesh::HexaArray>& hexahedra)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    vtkPointSet* output = vtkPointSet::SafeDownCast(reader->GetOutput());
    if(!output)
    {
        LOG(WARNING) << "VTKMeshReader::readAsGenericFormatData error: could not read with VTK reader.";
        return;
    }

    VTKMeshReader::copyVertices(output->GetPoints(), vertices);

    if(auto vtkMesh = reader->GetPolyDataOutput())
    {
        VTKMeshReader::copyCells<3>(vtkMesh->GetPolys(), triangles);
    }
    else if (auto vtkMesh = reader->GetUnstructuredGridOutput())
    {
        VTKMeshReader::copyCells<4>(vtkMesh->GetCells(), tetrahedra);
        //TODO : And hexahedra??
    }

    LOG(WARNING) << "VTKMeshReader::readAsGenericFormatData not finished.";
}

template<typename ReaderType>
void
VTKMeshReader::readAsAbstractPolyData(const std::string& filePath,
                                      std::vector<Vec3d>& vertices,
                                      std::vector<SurfaceMesh::TriangleArray>& triangles)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    vtkPolyData* vtkMesh = reader->GetOutput();
    if(!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshReader::readAsAbstractPolyData error: could not read with VTK reader.";
        return;
    }

    VTKMeshReader::copyVertices(vtkMesh->GetPoints(), vertices);
    VTKMeshReader::copyCells<3>(vtkMesh->GetPolys(), triangles);

    LOG(WARNING) << "VTKMeshReader::readAsAbstractPolyData not finished.";
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
}
