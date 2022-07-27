/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMeshIO.h"

namespace imstk
{
class AbstractCellMesh;
class HexahedralMesh;
class ImageData;
class LineMesh;
class SurfaceMesh;
class TetrahedralMesh;

///
/// \class VTKMeshIO
///
/// \brief Implements VTK read and write functions
///
class VTKMeshIO
{
public:
    VTKMeshIO() = default;
    virtual ~VTKMeshIO() = default;

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
    /// \brief Reads vtk unstructured grid. Drops cells that aren't of the last cell type.
    /// \return Returns TetrahedralMesh or HexahedralMesh
    ///
    template<typename ReaderType>
    static std::shared_ptr<AbstractCellMesh> readVtkUnstructuredGrid(const std::string& filePath);

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
} // namespace imstk