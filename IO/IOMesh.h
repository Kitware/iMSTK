// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------
#ifndef IOMESH_H
#define IOMESH_H

#include "Core/Factory.h"
#include <memory>

class IOMeshDelegate;
namespace Core{
    class BaseMesh;
}

///
/// \brief Mesh input/output class. This class is used to read/write meshes on
/// several formats. I currently used VTK, VegaFEM, Assimp and a custom reader
/// for .3ds files. Users can add more readers by implementing delegates for a
/// particular reader \see \IOMeshVTKDelegate, \IOMeshVegaDelegarte and
/// \IOMeshAssimpDelegate. Users and developers can extend this mesh reader to
/// any ther format by listing the format here in MeshFileType and implement a
/// delegate for the required format, \see \IOMeshAssimpDelegate,
/// \IOMeshVTKDelegate, \IOMesh3dsDelegate, \IOMeshVegaDelegate.
///
class IOMesh
{
public:
    ///
    /// \brief Enum class for the readers group. This enum is used to prioritize the io
    ///  delegates in the factory. For instance, VTK and Assimp both can read
    ///  stl files. You can use the contructor of IOMesh in order to give
    ///  highter (lower) priority to Assimp (or any other reader) to read .stl (or any
    ///  of the supported formats) formated files.
    ///
    enum ReaderGroup
    {
        VTK,
        Assimp,
        Vega,
        Other
    };

    ///
    /// \brief Constructor/Destructor
    ///
    IOMesh(const IOMesh::ReaderGroup &priorityGroup = ReaderGroup::VTK);
    ~IOMesh();

    ///
    /// \brief Enum class for the type of files this mesh io expect,
    ///         add more types here to extend the mesh io.
    ///
    enum class MeshFileType:int
    {
        VTK,
        VTU,
        VTP,
        OBJ,
        STL,
        PLY,
        VEG,
        ThreeDS,
        Unknown
    };

    ///
    /// \brief Read/Write meshes
    ///
    void read(const std::string &filePath);
    void write(const std::string &filePath);

    ///
    /// \brief Mesh accessors
    ///
    std::shared_ptr<Core::BaseMesh> getMesh();
    void setMesh(std::shared_ptr<Core::BaseMesh> newMesh);

    ///
    /// \brief Mesh accessors
    ///
    template<typename MeshType>
    std::shared_ptr<MeshType> getMeshAs()
    {
        return std::dynamic_pointer_cast<MeshType>(this->mesh);
    }

    ///
    /// \brief Filename accessors
    ///
    const std::string &getFileName() const;
    void setFilename(const std::string &filePath);

    ///
    /// \brief Returns the file type. This gets set to the correct typwhen calling \checkFileType.
    ///
    const MeshFileType &getFileType() const;

private:
    ///
    /// \brief Simple utility function for determine the extension of a file from its path.
    ///  There will be corner cases in which this function wont properly work.
    ///  eg "c:\program files\AppleGate.Net\readme"
    ///  TODO: Improve this fuction. Maybe using Boost filesystem library.
    ///
    IOMesh::MeshFileType getFileExtension() const;

private:
    // Storage for the mesh file name, used by delegates.
    std::string fileName;

    // Storage for the file type, used by delegates.
    MeshFileType fileType;

    // Mesh pointer.
    std::shared_ptr<Core::BaseMesh> mesh;

    // Handle delegation of readers.
    class DelegatorType;
    std::unique_ptr<DelegatorType> delegator;
};

#endif // MESHIO_H
