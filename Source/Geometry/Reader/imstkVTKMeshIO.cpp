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
#include "imstkGeometryUtilities.h"

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
#include "vtkPolyDataWriter.h"
#include "vtkDICOMImageReader.h"
#include "vtkNrrdReader.h"

#include "imstkSurfaceMesh.h"
#include "imstkLineMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"

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
    case MeshFileType::DCM:
    {
        return VTKMeshIO::readVtkImageDataDICOM(filePath);
    }
    case MeshFileType::NRRD:
    {
        return VTKMeshIO::readVtkImageData<vtkNrrdReader>(filePath);
    }
    default:
    {
        LOG(FATAL) << "VTKMeshIO::read error: file type not supported";
        return nullptr;
    }
    }
}

bool
VTKMeshIO::write(const std::shared_ptr<PointSet> imstkMesh, const std::string& filePath, const MeshFileType meshType)
{
    if (auto vMesh = std::dynamic_pointer_cast<VolumetricMesh>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::VTU:
            if (auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(vMesh))
            {
                VTKMeshIO::writeVtkUnstructuredGrid(*tetMesh.get(), filePath);
            }
            else if (auto hexMesh = std::dynamic_pointer_cast<HexahedralMesh>(vMesh))
            {
                VTKMeshIO::writeVtkUnstructuredGrid(*hexMesh.get(), filePath);
            }
            else
            {
                return false;
            }

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
            return VTKMeshIO::writeVtkPolyData<vtkXMLPolyDataWriter>(*sMesh.get(), filePath);
        case MeshFileType::STL:
            return VTKMeshIO::writeVtkPolyData<vtkSTLWriter>(*sMesh.get(), filePath);
        case MeshFileType::PLY:
            return VTKMeshIO::writeVtkPolyData<vtkPLYWriter>(*sMesh.get(), filePath);
        case MeshFileType::VTK:
            return VTKMeshIO::writeVtkPolyData<vtkPolyDataWriter>(*sMesh.get(), filePath);
        default:
            LOG(WARNING) << "VTKMeshIO::write error: file type not supported for surface mesh.";
            return false;
        }
    }
    else if (auto lMesh = std::dynamic_pointer_cast<LineMesh>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::VTK:
            return VTKMeshIO::writeVtkPolyData<vtkPolyDataWriter>(*lMesh.get(), filePath);
        case MeshFileType::VTP:
            return VTKMeshIO::writeVtkPolyData<vtkXMLPolyDataWriter>(*lMesh.get(), filePath);
        default:
            LOG(WARNING) << "vtkMeshIO::write error: file type not supported for line mesh.";
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

    if (vtkSmartPointer<vtkPolyData> vtkMesh = reader->GetPolyDataOutput())
    {
        return GeometryUtils::convertVtkPolyDataToSurfaceMesh(vtkMesh);
    }

    if (vtkUnstructuredGrid* vtkMesh = reader->GetUnstructuredGridOutput())
    {
        return GeometryUtils::convertVtkUnstructuredGridToVolumetricMesh(vtkMesh);
    }

    LOG(FATAL) << "VTKMeshIO::readVtkGenericFormatData error: could not read with VTK reader.";
    return nullptr;
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

    vtkSmartPointer<vtkPolyData> vtkMesh = triFilter->GetOutput();
    return GeometryUtils::convertVtkPolyDataToSurfaceMesh(vtkMesh);
}

template<typename WriterType>
bool
VTKMeshIO::writeVtkPolyData(const SurfaceMesh& imstkMesh, const std::string& filePath)
{
    vtkSmartPointer<vtkPolyData> vtkMesh = GeometryUtils::convertSurfaceMeshToVtkPolyData(imstkMesh);
    if (!vtkMesh)  //conversion unsuccessful
    {
        return false;
    }

    auto writer = vtkSmartPointer<WriterType>::New();
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Update();

    return true;
}

template<typename WriterType>
bool
VTKMeshIO::writeVtkPolyData(const LineMesh& imstkMesh, const std::string& filePath)
{
    vtkSmartPointer<vtkPolyData> vtkMesh = GeometryUtils::convertLineMeshToVtkPolyData(imstkMesh);
    vtkMesh->PrintSelf(std::cout, vtkIndent());
    if (!vtkMesh)  //conversion unsuccessful
    {
        return false;
    }

    auto writer = vtkSmartPointer<WriterType>::New();
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Update();

    return true;
}

template<typename ReaderType>
std::shared_ptr<VolumetricMesh>
VTKMeshIO::readVtkUnstructuredGrid(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    vtkSmartPointer<vtkUnstructuredGrid> vtkMesh = reader->GetOutput();
    return GeometryUtils::convertVtkUnstructuredGridToVolumetricMesh(vtkMesh);
}

template<typename ReaderType>
std::shared_ptr<ImageData>
VTKMeshIO::readVtkImageData(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    auto imageData = std::make_shared<ImageData>();
    imageData->initialize(reader->GetOutput());
    return imageData;
}

std::shared_ptr<ImageData>
VTKMeshIO::readVtkImageDataDICOM(const std::string& filePath)
{
    bool isDirectory;

    CHECK(MeshIO::fileExists(filePath, isDirectory)) << "VTKMeshIO::read error: file not found: " << filePath;

    if (!isDirectory)
    {
        return VTKMeshIO::readVtkImageData<vtkDICOMImageReader>(filePath);
    }

    auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(filePath.c_str());
    reader->Update();

    auto imageData = std::make_shared<ImageData>();
    imageData->initialize(reader->GetOutput());
    return imageData;
}

bool
VTKMeshIO::writeVtkUnstructuredGrid(const TetrahedralMesh& tetMesh, const std::string& filePath)
{
    auto vtkMesh = GeometryUtils::convertTetrahedralMeshToVtkUnstructuredGrid(tetMesh);

    if (!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshIO::writeVtkUnstructuredGrid error: conversion unsuccessful";
        return false;
    }

    auto writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Update();

    return true;
}

bool
VTKMeshIO::writeVtkUnstructuredGrid(const HexahedralMesh& hMesh, const std::string& filePath)
{
    auto vtkMesh = GeometryUtils::convertHexahedralMeshToVtkUnstructuredGrid(hMesh);

    if (!vtkMesh)
    {
        LOG(WARNING) << "VTKMeshIO::writeVtkUnstructuredGrid error: conversion unsuccessful";
        return false;
    }

    auto writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Update();

    return true;
}
} // imstk
