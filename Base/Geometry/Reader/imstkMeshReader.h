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

#ifndef imstkMeshReader_h
#define imstkMeshReader_h

#include <memory>

#include "imstkMesh.h"
#include "imstkSurfaceMesh.h"

namespace imstk {

class MeshReader
{
public:

    enum FileType
    {
        UNKNOWN,
        VTK,
        VTU,
        VTP,
        STL,
        PLY,
        OBJ,
        VEG
    };

    MeshReader() = default;
    ~MeshReader() = default;

    static std::shared_ptr<Mesh> read(const std::string& filePath);

    static std::shared_ptr<SurfaceMesh> createSurfaceMesh(const std::vector<Vec3d>& vertices,
                                                          const std::vector<SurfaceMesh::TriangleArray>& triangles,
                                                          const std::vector<Vec2f>& textCoords);

protected:

    static const FileType getFileType(const std::string& filePath);

};
}

#endif // ifndef imstkMeshReader_h
