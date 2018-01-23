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
#include "vtkXMLUnstructuredGridWriter.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPLYWriter.h"
#include "vtkSTLWriter.h"
#include "vtkFloatArray.h"
#include "vtkTriangleFilter.h"

#include "g3log/g3log.hpp"

namespace imstk
{
std::shared_ptr<PointSet>
VTKMeshIO::read(const std::string& filePath, MeshFileType meshType)
{
    switch (meshType)
    {
    case MeshFileType::VTK:
    {
        return VTKMeshIO::readVtkGenericFormatData<vtkGenericDataObjectReader>(filePath);
    }
    case MeshFileType::VTU:
    {
        return VTKMeshIO::readVtkUnstructuredGrid<vtkXMLUnstructuredGridReader>(filePath);
    }
    case MeshFileType::VTP:
    {
        return VTKMeshIO::readVtkPolyData<vtkXMLPolyDataReader>(filePath);
    }
    case MeshFileType::STL:
    {
        return VTKMeshIO::readVtkPolyData<vtkSTLReader>(filePath);
    }
    case MeshFileType::PLY:
    {
        return VTKMeshIO::readVtkPolyData<vtkPLYReader>(filePath);
    }
    case MeshFileType::OBJ:
    {
        return VTKMeshIO::readVtkPolyData<vtkOBJReader>(filePath);
    }
    default:
    {
        LOG(WARNING) << "VTKMeshIO::read error: file type not supported";
        return nullptr;
    }
    }
}

bool
VTKMeshIO::write(const std::shared_ptr<PointSet> imstkMesh, const std::string & filePath, const MeshFileType meshType)
{
    if (auto vMesh = std::dynamic_pointer_cast<VolumetricMesh>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::VTU:
            return VTKMeshIO::writeVtkUnstructuredGrid(vMesh, filePath);
        default:
            LOG(WARNING) << "VTKMeshIO::write error: file type not supported for volumetric mesh.";
            return false;
        }
    }
    else if (auto sMesh = std::dynamic_pointer_cast<SurfaceMesh>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::VTP:
            return VTKMeshIO::writeVtkPolyData<vtkXMLPolyDataWriter>(sMesh, filePath);
        case MeshFileType::STL:
            return VTKMeshIO::writeVtkPolyData<vtkSTLWriter>(sMesh, filePath);
        case MeshFileType::PLY:
            return VTKMeshIO::writeVtkPolyData<vtkPLYWriter>(sMesh, filePath);
        default:
            LOG(WARNING) << "VTKMeshIO::write error: file type not supported for surface mesh.";
            return false;
        }
    }
    else
    {
        LOG(WARNING) << "VTKMeshIO::write error: the provided mesh is not a surface or volumetric mesh.";
        return false;
    }
}

template<typename ReaderType>
std::shared_ptr<PointSet>
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

template<typename WriterType>
bool
VTKMeshIO::writeVtkPolyData(const std::shared_ptr<SurfaceMesh> imstkMesh, const std::string & filePath)
{
    vtkPolyData* vtkMesh = convertSurfaceMeshToVtkPolyData(imstkMesh);
    if (!vtkMesh) //conversion unsuccessful
    {
        return false;
    }

    auto writer = vtkSmartPointer<WriterType>::New();
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Update();
    vtkMesh->Delete();

    return true;
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

bool
VTKMeshIO::writeVtkUnstructuredGrid(const std::shared_ptr<VolumetricMesh> imstkMesh, const std::string & filePath)
{
    auto tMesh = std::dynamic_pointer_cast<TetrahedralMesh>(imstkMesh);
    auto hMesh = std::dynamic_pointer_cast<HexahedralMesh>(imstkMesh);
    vtkUnstructuredGrid* vtkMesh = nullptr;
    if (tMesh)
    {
        vtkMesh = convertTetrahedralMeshToVtkUnstructuredGrid(tMesh);
    }
    else if (hMesh)
    {
        vtkMesh = convertHexahedralMeshToVtkUnstructuredGrid(hMesh);
    }
    else
    {
        LOG(WARNING) << "VTKMeshIO::writeVtkUnstructuredGrid error: mesh is neither tetrahedral nor hexahedral";
        return false;
    }

    if (!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshIO::writeVtkUnstructuredGrid error: conversion unsuccessful";
        return false;
    }

    auto writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Update();
    vtkMesh->Delete();

    return true;
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
    VTKMeshIO::copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    std::vector<SurfaceMesh::TriangleArray> triangles;
    VTKMeshIO::copyCellsFromVtk<3>(vtkMesh->GetPolys(), triangles);

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
    {
        if (auto tcoords = pointData->GetTCoords())
        {
            mesh->setDefaultTCoords(tcoords->GetName());
        }
    }

    return mesh;
}

vtkPolyData*
VTKMeshIO::convertSurfaceMeshToVtkPolyData(std::shared_ptr<SurfaceMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    VTKMeshIO::copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    VTKMeshIO::copyCellsToVtk<3>(imstkMesh->getTrianglesVertices(), polys.Get());

    vtkPolyData *polydata = vtkPolyData::New();
    polydata->SetPoints(points);
    polydata->SetPolys(polys);
    return polydata;
}

vtkUnstructuredGrid*
VTKMeshIO::convertTetrahedralMeshToVtkUnstructuredGrid(std::shared_ptr<TetrahedralMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    VTKMeshIO::copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkSmartPointer<vtkCellArray> tetras = vtkSmartPointer<vtkCellArray>::New();
    VTKMeshIO::copyCellsToVtk<4>(imstkMesh->getTetrahedraVertices(), tetras.Get());

    vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_TETRA, tetras);
    return ug;
}

vtkUnstructuredGrid*
VTKMeshIO::convertHexahedralMeshToVtkUnstructuredGrid(std::shared_ptr<HexahedralMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    VTKMeshIO::copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkSmartPointer<vtkCellArray> bricks = vtkSmartPointer<vtkCellArray>::New();
    VTKMeshIO::copyCellsToVtk<8>(imstkMesh->getHexahedraVertices(), bricks.Get());

    vtkUnstructuredGrid *ug = vtkUnstructuredGrid::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_HEXAHEDRON, bricks);
    return ug;
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
    VTKMeshIO::copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    int cellType = vtkMesh->GetCellType(vtkMesh->GetNumberOfCells()-1);
    if( cellType == VTK_TETRA )
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        VTKMeshIO::copyCellsFromVtk<4>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_shared<TetrahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else if( cellType == VTK_HEXAHEDRON )
    {
        std::vector<HexahedralMesh::HexaArray> cells;
        VTKMeshIO::copyCellsFromVtk<8>(vtkMesh->GetCells(), cells);

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
VTKMeshIO::copyVerticesFromVtk(vtkPoints* points, StdVectorOfVec3d& vertices)
{
    if(!points)
    {
        LOG(WARNING) << "VTKMeshIO::copyVerticesFromVtk error: No points found.";
        return;
    }

    vertices.reserve(points->GetNumberOfPoints());
    for(vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
        double pos[3];
        points->GetPoint(i, pos);
        vertices.emplace_back(pos[0], pos[1], pos[2]);
    }
}

void VTKMeshIO::copyVerticesToVtk(const StdVectorOfVec3d & vertices, vtkPoints * points)
{
    if (!points)
    {
        LOG(WARNING) << "VTKMeshIO::copyVerticesToVtk error: No points found.";
        return;
    }

    points->SetNumberOfPoints(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++)
    {
        points->SetPoint(i, vertices[i][0], vertices[i][1], vertices[i][2]);
    }
}

template<size_t dim>
void VTKMeshIO::copyCellsToVtk(const std::vector<std::array<size_t, dim>>& cells, vtkCellArray * vtkCells)
{
    if (!vtkCells)
    {
        LOG(WARNING) << "VTKMeshIO::copyCellsToVtk error: No cells found.";
        return;
    }

    for (size_t i = 0; i < cells.size(); i++)
    {
        vtkCells->InsertNextCell(dim);
        for (size_t k = 0; k < dim; k++)
        {
            vtkCells->InsertCellPoint(cells[i][k]);
        }
    }
}

template<size_t dim>
void
VTKMeshIO::copyCellsFromVtk(vtkCellArray* vtkCells, std::vector<std::array<size_t,dim>>& cells)
{
    if(!vtkCells)
    {
        LOG(WARNING) << "VTKMeshIO::copyCellsFromVtk error: No cells found.";
        return;
    }

    cells.reserve(vtkCells->GetNumberOfCells());
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

    for (int i = 0; i < pointData->GetNumberOfArrays(); ++i)
    {
        vtkDataArray* array = pointData->GetArray(i);
        std::string name = array->GetName();
        int nbrOfComp = array->GetNumberOfComponents();
        vtkIdType nbrOfTuples = array->GetNumberOfTuples();
        StdVectorOfVectorf data;
        for(vtkIdType j = 0; j < nbrOfTuples; ++j)
        {
            double* tupleData = new double [nbrOfComp];
            array->GetTuple(j, tupleData);
            Vectorf tuple(nbrOfComp);
            for (int k = 0; k < nbrOfComp; k++)
            {
                tuple[k] = tupleData[k];
            }
            data.push_back(tuple);
        }
        dataMap[name] = data;
    }
}
} // imstk
