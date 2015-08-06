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


#ifndef MESHIO_H
#define MESHIO_H

#include <memory>
#include <unordered_map>

#include "InputOutput/IOMeshDelegate.h"
#include "Core/Factory.h"
#include "Core/BaseMesh.h"

///
/// \brief Mesh input/output class. This class is used to read meshes on several formats.
///  Users can add more readers by implementing delegates for a particular reader
///     \see \VTKMeshDelegate, \VegaMeshDelegarte and \AssimpMeshDelegate
///
class IOMesh
{
    typedef std::function<std::shared_ptr<IOMeshDelegate>(std::shared_ptr<IOMesh>)> DelegatorType;
public:
    ///
    /// \brief Enum class for the type of files this mesh io expect,
    ///         add more types here to extend the mesh io.
    ///
    enum class MeshFileType
    {
        VTK,
        VTU,
        VTP,
        OBJ,
        STL,
        PLY,
        VEG,
        Unknown
    };


    ///
    /// \brief Enum class for the readers group. Used to prioritize the io delegates in the factory.
    ///
    enum class ReaderGroup : int
    {
        VTK,
        Assimp,
        Vega,
        Other
    };

    ///
    /// \brief Constructor/Destructors
    ///
    IOMesh(ReaderGroup priorityGroup = ReaderGroup::VTK);
    ~IOMesh();

    ///
    /// \brief Read/Write meshes
    ///
    void read(const std::string &filePath);
    void write(const std::string &filePath);

    ///
    /// \brief Simplistic function for figuring out the extension of a file from its path.
    ///  There will be corner cases in which this function wont properly work.
    ///  eg "c:\program files\AppleGate.Net\readme"
    ///
    void checkFileType();

    ///
    /// \brief Mesh accessors
    ///
    std::shared_ptr<Core::BaseMesh> getMesh();
    void setMesh(std::shared_ptr<Core::BaseMesh> newMesh);

    ///
    /// \brief Filename accessors
    ///
    const std::string &getFilename() const;
    void setFilename(const std::string &filePath);

    ///
    /// \brief Returns the file type. This gets set to the correct typwhen calling \checkFileType.
    ///
    const MeshFileType &getFileType() const;

private:
    // Storage for the mesh file name, used by delegates.
    std::string fileName;

    // Storage for the file type, used by delegates.
    MeshFileType fileType;

    // Mesh pointer.
    std::shared_ptr<Core::BaseMesh> mesh;

    // Map file types with reader delegates.
    std::unordered_map<MeshFileType,DelegatorType> delegatorList;
};

#endif // MESHIO_H
