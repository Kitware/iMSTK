/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <memory>
#include <string>

namespace imstk
{
class PointSet;

///
/// \brief Enumeration the mesh file type
///
enum class MeshFileType
{
    UNKNOWN,
    VTK,
    VTU,
    VTP,
    STL,
    PLY,
    OBJ,
    DAE,
    FBX,
    _3DS,
    VEG,
    MSH,
    NRRD,
    DCM,
    NII,
    MHD,
    JPG,
    PNG,
    BMP
};

///
/// \class MeshIO
///
/// \brief Mesh data IO
///
class MeshIO
{
public:
    MeshIO() = default;
    virtual ~MeshIO() = default;

    ///
    /// \brief Read external file
    ///
    static std::shared_ptr<PointSet> read(const std::string& filePath);

    template<typename T>
    static std::shared_ptr<T> read(const std::string& filePath) { return std::dynamic_pointer_cast<T>(read(filePath)); }

    ///
    /// \brief Write external file
    ///
    static bool write(const std::shared_ptr<imstk::PointSet> imstkMesh, const std::string& filePath);

    ///
    /// \brief Returns true if the file exists, else false.
    /// Also sets isDirectory to true if the path is a directory, else false.
    ///
    static bool fileExists(const std::string& file, bool& isDirectory);

    ///
    /// \brief Returns the type of the file
    ///
    static const MeshFileType getFileType(const std::string& filePath);
};
} // namespace imstk