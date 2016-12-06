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

#ifndef imstkVTKMeshIO_h
#define imstkVTKMeshIO_h

#include <memory>

#include "vtkUnstructuredGrid.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"

#include "imstkMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"

namespace imstk
{

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
    static void copyVertices(vtkPoints* points, StdVectorOfVec3d& vertices);

    ///
    /// \brief
    ///
    template<size_t dim>
    static void copyCells(vtkCellArray* vtkCells, std::vector<std::array<size_t,dim>>& cells);

    ///
    /// \brief
    ///
    static void copyPointData(vtkPointData* pointData, std::map<std::string, StdVectorOfVectorf>& dataMap);

};

} // imstk

#endif // ifndef imstkVTKMeshIO_h
