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
#include "imstkHexahedralMesh.h"
#include "imstkImageData.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include <vtkBMPReader.h>
#include <vtkBMPWriter.h>
#include <vtkDICOMImageReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkGenericDataObjectWriter.h>
#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkJPEGWriter.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkNrrdReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>

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
    case MeshFileType::NII:
    {
        return VTKMeshIO::readVtkImageDataNIFTI(filePath);
    }
    case MeshFileType::MHD:
    {
        return VTKMeshIO::readVtkImageData<vtkMetaImageReader>(filePath);
    }
    case MeshFileType::PNG:
    {
        return VTKMeshIO::readVtkImageData<vtkPNGReader>(filePath);
    }
    case MeshFileType::JPG:
    {
        return VTKMeshIO::readVtkImageData<vtkJPEGReader>(filePath);
    }
    case MeshFileType::BMP:
    {
        return VTKMeshIO::readVtkImageData<vtkBMPReader>(filePath);
    }
    default:
    {
        LOG(FATAL) << "Error: file type not supported for input " << filePath;
        return nullptr;
    }
    }
}

bool
VTKMeshIO::write(const std::shared_ptr<PointSet> imstkMesh, const std::string& filePath, const MeshFileType meshType)
{
    if (auto imgMesh = std::dynamic_pointer_cast<ImageData>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::NII:
        {
            return VTKMeshIO::writeVtkImageDataNIFTI(imgMesh, filePath);
        }
        case MeshFileType::MHD:
        {
            return VTKMeshIO::writeMetaImageData(imgMesh, filePath);
        }
        case MeshFileType::PNG:
        {
            return VTKMeshIO::writeVtkImageData<vtkPNGWriter>(imgMesh, filePath);
        }
        case MeshFileType::JPG:
        {
            return VTKMeshIO::writeVtkImageData<vtkJPEGWriter>(imgMesh, filePath);
        }
        case MeshFileType::BMP:
        {
            return VTKMeshIO::writeVtkImageData<vtkBMPWriter>(imgMesh, filePath);
        }
        default:
            LOG(WARNING) << "Error: file type not supported for volumetric mesh. Target path supplied:" << filePath;
            return false;
        }
    }
    else if (auto vMesh = std::dynamic_pointer_cast<VolumetricMesh>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::VTU:
            if (auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(vMesh))
            {
                return VTKMeshIO::writeVtkUnstructuredGrid<vtkXMLUnstructuredGridWriter>(tetMesh, filePath);
            }
            else if (auto hexMesh = std::dynamic_pointer_cast<HexahedralMesh>(vMesh))
            {
                return VTKMeshIO::writeVtkUnstructuredGrid<vtkXMLUnstructuredGridWriter>(hexMesh, filePath);
            }
            else
            {
                return false;
            }

        case MeshFileType::VTK:
            if (auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(vMesh))
            {
                return VTKMeshIO::writeVtkUnstructuredGrid<vtkGenericDataObjectWriter>(tetMesh, filePath);
            }
            else if (auto hexMesh = std::dynamic_pointer_cast<HexahedralMesh>(vMesh))
            {
                return VTKMeshIO::writeVtkUnstructuredGrid<vtkGenericDataObjectWriter>(hexMesh, filePath);
            }
            else
            {
                return false;
            }

        default:
            LOG(WARNING) << "Error: file type not supported for volumetric mesh. Target path supplied:" << filePath;
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
        case MeshFileType::VTK:
            return VTKMeshIO::writeVtkPolyData<vtkPolyDataWriter>(sMesh, filePath);
        default:
            LOG(WARNING) << "Error: file type not supported for surface mesh. Target path supplied:" << filePath;
            return false;
        }
    }
    else if (auto lMesh = std::dynamic_pointer_cast<LineMesh>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::VTK:
            return VTKMeshIO::writeVtkPolyData<vtkPolyDataWriter>(lMesh, filePath);
        case MeshFileType::VTP:
            return VTKMeshIO::writeVtkPolyData<vtkXMLPolyDataWriter>(lMesh, filePath);
        default:
            LOG(WARNING) << "Error: file type not supported for line mesh. Target path supplied:" << filePath;
            return false;
        }
    }
    else if (auto ptMesh = std::dynamic_pointer_cast<PointSet>(imstkMesh))
    {
        switch (meshType)
        {
        case MeshFileType::VTK:
            return VTKMeshIO::writeVtkPointSet<vtkGenericDataObjectWriter>(ptMesh, filePath);
        default:
            LOG(WARNING) << "Error: file type not supported for point mesh. Target path supplied:" << filePath;
            return false;
        }
    }
    else
    {
        LOG(WARNING) << "Error: the provided mesh is not a surface or volumetric mesh. Target path supplied:" << filePath;
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
        // Try to convert to surface mesh, if no elements exist try reading as a line mesh
        std::shared_ptr<SurfaceMesh> surfMesh = GeometryUtils::copyToSurfaceMesh(vtkMesh);
        if (surfMesh->getNumTriangles() > 0)
        {
            return surfMesh;
        }
        std::shared_ptr<LineMesh> lineMesh = GeometryUtils::copyToLineMesh(vtkMesh);
        if (lineMesh->getNumLines() > 0)
        {
            return lineMesh;
        }
        return GeometryUtils::copyToPointSet(vtkMesh);
    }

    if (vtkUnstructuredGrid* vtkMesh = reader->GetUnstructuredGridOutput())
    {
        return GeometryUtils::copyToVolumetricMesh(vtkMesh);
    }

    LOG(FATAL) << "Error: could not read with VTK reader for input " << filePath;
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
    return GeometryUtils::copyToSurfaceMesh(vtkMesh);
}

template<typename WriterType>
bool
VTKMeshIO::writeVtkImageData(const std::shared_ptr<ImageData> imstkMesh, const std::string& filePath)
{
    vtkSmartPointer<vtkImageData> vtkMesh = GeometryUtils::copyToVtkImageData(imstkMesh);
    if (!vtkMesh)
    {
        return false;
    }

    auto writer = vtkSmartPointer<WriterType>::New();
    if (vtkMesh->GetDimensions()[2] == 1)
    {
        writer->SetFileDimensionality(2);
    }
    else
    {
        writer->SetFileDimensionality(3);
    }
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Write();

    return true;
}

template<typename WriterType>
bool
VTKMeshIO::writeVtkPolyData(std::shared_ptr<SurfaceMesh> imstkMesh, const std::string& filePath)
{
    vtkSmartPointer<vtkPolyData> vtkMesh = GeometryUtils::copyToVtkPolyData(imstkMesh);
    if (!vtkMesh)
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
VTKMeshIO::writeVtkPolyData(std::shared_ptr<LineMesh> imstkMesh, const std::string& filePath)
{
    vtkSmartPointer<vtkPolyData> vtkMesh = GeometryUtils::copyToVtkPolyData(imstkMesh);
    if (!vtkMesh)
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
VTKMeshIO::writeVtkPointSet(const std::shared_ptr<PointSet> imstkMesh, const std::string& filePath)
{
    vtkSmartPointer<vtkPointSet> vtkMesh = GeometryUtils::copyToVtkPointSet(imstkMesh);
    if (!vtkMesh)
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
    return GeometryUtils::copyToVolumetricMesh(vtkMesh);
}

std::shared_ptr<ImageData>
VTKMeshIO::readVtkImageDataDICOM(const std::string& filePath)
{
    bool isDirectory;

    CHECK(MeshIO::fileExists(filePath, isDirectory)) << "Error: file " << filePath << " not found!";

    if (!isDirectory)
    {
        return VTKMeshIO::readVtkImageData<vtkDICOMImageReader>(filePath);
    }

    auto reader = vtkSmartPointer<vtkDICOMImageReader>::New();
    reader->SetDirectoryName(filePath.c_str());
    reader->Update();

    std::shared_ptr<ImageData> imageData(
        std::move(GeometryUtils::copyToImageData(reader->GetOutput())));
    return imageData;
}

template<typename WriterType>
bool
VTKMeshIO::writeVtkUnstructuredGrid(std::shared_ptr<TetrahedralMesh> tetMesh, const std::string& filePath)
{
    auto vtkMesh = GeometryUtils::copyToVtkUnstructuredGrid(tetMesh);

    if (!vtkMesh)
    {
        LOG(WARNING) << "Error: conversion unsuccessful. Target path supplied:" << filePath;
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
VTKMeshIO::writeVtkUnstructuredGrid(std::shared_ptr<HexahedralMesh> hMesh, const std::string& filePath)
{
    auto vtkMesh = GeometryUtils::copyToVtkUnstructuredGrid(hMesh);

    if (!vtkMesh)
    {
        LOG(WARNING) << "Error: conversion unsuccessful. Target path supplied:" << filePath;
        return false;
    }

    auto writer = vtkSmartPointer<WriterType>::New();
    writer->SetInputData(vtkMesh);
    writer->SetFileName(filePath.c_str());
    writer->Update();

    return true;
}

template<typename ReaderType>
std::shared_ptr<ImageData>
VTKMeshIO::readVtkImageData(const std::string& filePath)
{
    auto reader = vtkSmartPointer<ReaderType>::New();
    reader->SetFileName(filePath.c_str());
    reader->Update();

    std::shared_ptr<ImageData> imageData(std::move(GeometryUtils::copyToImageData(reader->GetOutput())));
    return imageData;
}

std::shared_ptr<ImageData>
VTKMeshIO::readVtkImageDataNIFTI(const std::string& filePath)
{
    auto reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
    reader->SetFileName(filePath.c_str());
    reader->SetFileDimensionality(3);
    reader->Update();

    std::shared_ptr<ImageData> imageData(std::move(GeometryUtils::copyToImageData(reader->GetOutput())));
    return imageData;
}

bool
VTKMeshIO::writeVtkImageDataNIFTI(std::shared_ptr<ImageData> imageData, const std::string& filePath)
{
    // Copy instead of couple for thread safety
    auto vtkMesh = GeometryUtils::copyToVtkImageData(imageData);
    if (!vtkMesh)
    {
        LOG(WARNING) << "Error: conversion unsuccessful. Target path supplied:" << filePath;
        return false;
    }

    auto writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
    writer->SetFileName(filePath.c_str());
    if (vtkMesh->GetDimensions()[2] == 1)
    {
        writer->SetFileDimensionality(2);
    }
    else
    {
        writer->SetFileDimensionality(3);
    }
    writer->SetInputData(vtkMesh);
    writer->Update();
    return true;
}

bool
VTKMeshIO::writeMetaImageData(std::shared_ptr<ImageData> imageData, const std::string& filePath)
{
    vtkSmartPointer<vtkImageData> vtkImage = GeometryUtils::copyToVtkImageData(imageData);
    if (!vtkImage)
    {
        LOG(WARNING) << "Error: conversion unsuccessful. Target path supplied:" << filePath;
        return false;
    }

    auto writer = vtkSmartPointer<vtkMetaImageWriter>::New();
    if (vtkImage->GetDimensions()[2] == 1)
    {
        writer->SetFileDimensionality(2);
    }
    else
    {
        writer->SetFileDimensionality(3);
    }
    writer->SetInputData(vtkImage);
    writer->SetFileName(filePath.c_str());
    writer->SetRAWFileName((filePath + ".raw").c_str());
    writer->Write();
    return true;
}
} // namespace imstk