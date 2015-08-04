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

#include "Core/BaseMesh.h"
#include "InputOutput/ReaderDelegate.h"

class MeshIO : std::enable_shared_from_this<MeshIO>
{
public:
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
    MeshIO();
    ~MeshIO();

    ///
    /// \brief Read/Write meshes
    ///
    void read(const std::string &filename);
    void write(const std::string &filename);

    ///
    /// \brief Simplistic function for figuring out the extension of a file from its path.
    ///  There will be corner cases in which this function wont properly work.
    ///  eg "c:\program files\AppleGate.Net\readme"
    ///
    MeshFileType checkFileType(const std::string &filename);

    std::shared_ptr<Core::BaseMesh> getMesh();

    void setReaderDelegate(std::shared_ptr<ReaderDelegate> delegate);

private:
    class MeshReader;
    std::unique_ptr<MeshReader> reader;
    std::shared_ptr<Core::BaseMesh> mesh;
    std::shared_ptr<ReaderDelegate> readerDelegate;
};

#endif // MESHIO_H
