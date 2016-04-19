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

#ifndef imstkVTKMeshReader_h
#define imstkVTKMeshReader_h

#include <memory>

#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
//#include "vtkFieldData.h"

#include "imstkMeshReader.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"

namespace imstk {

class VTKMeshReader
{
public:

    VTKMeshReader() = default;
    ~VTKMeshReader() = default;

    static std::shared_ptr<Mesh> read(const std::string& filePath, MeshReader::FileType meshType);

protected:

    template<typename ReaderType>
    static std::shared_ptr<Mesh> readAsGenericFormatData(const std::string& filePath);

    template<typename ReaderType>
    static std::shared_ptr<SurfaceMesh> readAsAbstractPolyData(const std::string& filePath);

    static void copyVertices(vtkPoints* points, std::vector<Vec3d>& vertices);

    template<size_t dim>
    static void copyCells(vtkCellArray* vtkCells, std::vector<std::array<size_t,dim>>& cells);

    static void copyTextureCoordinates(vtkPointData* pointData, std::vector<Vec2f>& textCoords);

    //static void copyData(vtkFieldData* fields, ...);
};
}

#endif // ifndef imstkVTKMeshReader_h
