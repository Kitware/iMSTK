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

#include "imstkMath.h"
#include <memory>
#include <vtkSmartPointer.h>

class vtkCellArray;
class vtkPolyData;
class vtkPointData;
class vtkPoints;
class vtkUnstructuredGrid;

namespace imstk
{
class HexahedralMesh;
class LineMesh;
class SurfaceMesh;
class TetrahedralMesh;
class VolumetricMesh;

namespace GeometryUtils
{
    ///
    /// \brief Converts vtk polydata into a imstk surface mesh
    ///
    std::unique_ptr<SurfaceMesh> convertVtkPolyDataToSurfaceMesh(const vtkSmartPointer<vtkPolyData> vtkMesh);

    ///
    /// \brief Converts vtk polydata into a imstk surface mesh
    ///
    std::unique_ptr<LineMesh> convertVtkPolyDataToLineMesh(vtkSmartPointer<vtkPolyData> vtkMesh);

    ///
    /// \brief Converts imstk surface mesh into a vtk polydata
    ///
    vtkSmartPointer<vtkPolyData> convertSurfaceMeshToVtkPolyData(const SurfaceMesh& imstkMesh);

    ///
    /// \brief Converts imstk line mesh into a vtk polydata
    ///
    vtkSmartPointer<vtkPolyData> convertLineMeshToVtkPolyData(const LineMesh& imstkMesh);

    ///
    /// \brief Converts imstk tetrahedral mesh into a vtk unstructured grid
    ///
    vtkSmartPointer<vtkUnstructuredGrid> convertTetrahedralMeshToVtkUnstructuredGrid(const TetrahedralMesh& imstkMesh);

    ///
    /// \brief Converts imstk hexahedral mesh into a vtk unstructured grid
    ///
    vtkSmartPointer<vtkUnstructuredGrid> convertHexahedralMeshToVtkUnstructuredGrid(const HexahedralMesh& imstkMesh);

    ///
    /// \brief
    ///
    std::unique_ptr<VolumetricMesh> convertVtkUnstructuredGridToVolumetricMesh(vtkUnstructuredGrid* const vtkMesh);

    ///
    /// \brief
    ///
    void copyVerticesFromVtk(vtkPoints* const points, StdVectorOfVec3d& vertices);

    ///
    /// \brief Copies vertices from imstk structure to VTK one
    ///
    void copyVerticesToVtk(const StdVectorOfVec3d& vertices, vtkPoints* points);

    ///
    /// \brief Copies cells of the given dimension from imstk structure to VTK one
    ///
    template<size_t dim>
    void copyCellsToVtk(const std::vector<std::array<size_t, dim>>& cells, vtkCellArray* vtkCells);

    ///
    /// \brief
    ///
    template<size_t dim>
    void copyCellsFromVtk(vtkCellArray* vtkCells, std::vector<std::array<size_t, dim>>& cells);

    ///
    /// \brief
    ///
    void copyPointDataFromVtk(vtkPointData* const pointData, std::map<std::string, StdVectorOfVectorf>& dataMap);

    ///
    /// \brief Appends two surface meshes
    ///
    std::unique_ptr<SurfaceMesh> combineSurfaceMesh(const SurfaceMesh& surfaceMesh1, const SurfaceMesh& surfaceMesh2);

    ///
    /// \brief Converts an imstk SurfaceMesh to a LineMesh, removing duplicate edges. Cell indices not preserved
    ///
    std::unique_ptr<LineMesh> surfaceMeshToLineMesh(const SurfaceMesh& surfaceMesh);

    ///
    /// \brief Smooths a SurfaceMesh using laplacian smoothening
    ///
    std::unique_ptr<SurfaceMesh> smoothSurfaceMesh(const SurfaceMesh& surfaceMesh,
        const int numberOfIterations = 20, const double relaxationFactor = 0.01,
        const double convergence = 0.0, const double featureAngle = 45.0,
        const double edgeAngle = 15.0, const bool featureEdgeSmoothing = false,
        const bool boundarySmoothing = true);

    ///
    /// \brief Sub-divdes a SurfaceMesh using linear subdivision
    ///
    std::unique_ptr<SurfaceMesh> linearSubdivideSurfaceMesh(const SurfaceMesh& surfaceMesh, const int numSubdivisions=1);

    ///
    /// \brief Sub-divides a SurfaceMesh using loop subdivision
    ///
    std::unique_ptr<SurfaceMesh> loopSubdivideSurfaceMesh(const SurfaceMesh& surfaceMesh, const int numSubdivisions=1);
};
}
