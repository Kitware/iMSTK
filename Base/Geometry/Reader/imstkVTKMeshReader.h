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

#include "vtkUnstructuredGrid.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"

#include "imstkMeshReader.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"

namespace imstk {

///
/// \class VTKMeshReader
///
/// \brief
///
class VTKMeshReader
{
public:

    VTKMeshReader() = default;
    ~VTKMeshReader() = default;

    ///
    /// \brief
    ///
    static std::shared_ptr<Mesh> read(const std::string& filePath, MeshFileType meshType);

protected:

    ///
    /// \brief
    ///
    template<typename ReaderType>
    static std::shared_ptr<Mesh> readVtkGenericFormatData(const std::string& filePath);

    ///
    /// \brief
    ///
    template<typename ReaderType>
    static std::shared_ptr<SurfaceMesh> readVtkPolyData(const std::string& filePath);

    ///
    /// \brief
    ///
    template<typename ReaderType>
    static std::shared_ptr<VolumetricMesh> readVtkUnstructuredGrid(const std::string& filePath);

    ///
    /// \brief
    ///
    static std::shared_ptr<SurfaceMesh> convertVtkPolyDataToSurfaceMesh(vtkPolyData* vtkMesh);

    ///
    /// \brief
    ///
    static std::shared_ptr<VolumetricMesh> convertVtkUnstructuredGridToVolumetricMesh(vtkUnstructuredGrid* vtkMesh);

    ///
    /// \brief
    ///
    static void copyVertices(vtkPoints* points, std::vector<Vec3d>& vertices);

    ///
    /// \brief
    ///
    template<size_t dim>
    static void copyCells(vtkCellArray* vtkCells, std::vector<std::array<size_t,dim>>& cells);

    ///
    /// \brief
    ///
    static void copyPointData(vtkPointData* pointData, std::map<std::string, std::vector<VecNf>>& dataMap);

};
}

#endif // ifndef imstkVTKMeshReader_h
