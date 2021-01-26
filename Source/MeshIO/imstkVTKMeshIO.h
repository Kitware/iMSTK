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

#pragma once

#include "imstkMeshIO.h"

namespace imstk
{
class HexahedralMesh;
class ImageData;
class LineMesh;
class SurfaceMesh;
class TetrahedralMesh;
class VolumetricMesh;

///
/// \class VTKMeshIO
///
/// \brief
///
class VTKMeshIO
{
public:

    ///
    /// \brief Default constructor
    ///
    VTKMeshIO() = default;

    ///
    /// \brief Default destructor
    ///
    ~VTKMeshIO() = default;

    ///
    /// \brief
    ///
    static std::shared_ptr<PointSet> read(const std::string& filePath, MeshFileType meshType);

    ///
    /// \brief Writes the given mesh to the specified file path.
    ///
    static bool write(const std::shared_ptr<PointSet> imstkMesh, const std::string& filePath, const MeshFileType meshType);

protected:

    ///
    /// \brief
    ///
    template<typename ReaderType>
    static std::shared_ptr<PointSet> readVtkGenericFormatData(const std::string& filePath);

    ///
    /// \brief
    ///
    template<typename ReaderType>
    static std::shared_ptr<SurfaceMesh> readVtkPolyData(const std::string& filePath);

    ///
    /// \brief Writes the given image data to given file path using the provided writer type
    ///
    template<typename WriterType>
    static bool writeVtkImageData(const std::shared_ptr<ImageData> imstkMesh, const std::string& filePath);

    ///
    /// \brief Writes the given surface mesh to given file path using the provided writer type
    ///
    template<typename WriterType>
    static bool writeVtkPolyData(const std::shared_ptr<SurfaceMesh> imstkMesh, const std::string& filePath);

    ///
    /// \brief Writes the given line mesh to given file path using the provided writer type
    ///
    template<typename WriterType>
    static bool writeVtkPolyData(const std::shared_ptr<LineMesh> imstkMesh, const std::string& filePath);

    ///
    /// \brief Writes the given point set to given file path using the provided writer type
    ///
    template<typename WriterType>
    static bool writeVtkPointSet(const std::shared_ptr<PointSet> imstkMesh, const std::string& filePath);

    ///
    /// \brief Writes the given volumetric mesh to given file path
    ///
    template<typename WriterType>
    static bool writeVtkUnstructuredGrid(const std::shared_ptr<TetrahedralMesh> tetMesh, const std::string& filePath);
    template<typename WriterType>
    static bool writeVtkUnstructuredGrid(const std::shared_ptr<HexahedralMesh> hMesh, const std::string& filePath);

    ///
    /// \brief Reads vtk unstructured grid
    ///
    template<typename ReaderType>
    static std::shared_ptr<VolumetricMesh> readVtkUnstructuredGrid(const std::string& filePath);

    ///
    /// \brief Reads vtk image data
    ///
    template<typename ReaderType>
    static std::shared_ptr<ImageData> readVtkImageData(const std::string& filePath);

    ///
    /// \brief TODO
    ///
    static std::shared_ptr<ImageData> readVtkImageDataDICOM(const std::string& filePath);

    ///
    /// \brief Reads nifti/nii format image data
    ///
    static std::shared_ptr<ImageData> readVtkImageDataNIFTI(const std::string& filePath);

    ///
    /// \brief Write nifti/nii format image data
    ///
    static bool writeVtkImageDataNIFTI(std::shared_ptr<ImageData> imageData, const std::string& filePath);

    ///
    /// \brief Write meta/mhd format image data
    /// 
    static bool writeMetaImageData(std::shared_ptr<ImageData> imageData, const std::string& filePath);
};
} // imstk
