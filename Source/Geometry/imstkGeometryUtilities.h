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

class GeometryUtils
{
public:
    ///
    /// \brief Converts vtk polydata into a imstk surface mesh
    ///
    static std::shared_ptr<SurfaceMesh> convertVtkPolyDataToSurfaceMesh(vtkSmartPointer<vtkPolyData> vtkMesh);

    ///
    /// \brief Converts vtk polydata into a imstk surface mesh
    ///
    static std::shared_ptr<LineMesh> convertVtkPolyDataToLineMesh(vtkSmartPointer<vtkPolyData> vtkMesh);

    ///
    /// \brief Converts imstk surface mesh into a vtk polydata
    ///
    static vtkSmartPointer<vtkPolyData> convertSurfaceMeshToVtkPolyData(std::shared_ptr<SurfaceMesh> imstkMesh);

    ///
    /// \brief Converts imstk line mesh into a vtk polydata
    ///
    static vtkSmartPointer<vtkPolyData> convertLineMeshToVtkPolyData(std::shared_ptr<LineMesh> imstkMesh);

    ///
    /// \brief Converts imstk tetrahedral mesh into a vtk unstructured grid
    ///
    static vtkSmartPointer<vtkUnstructuredGrid> convertTetrahedralMeshToVtkUnstructuredGrid(std::shared_ptr<TetrahedralMesh> imstkMesh);

    ///
    /// \brief Converts imstk hexahedral mesh into a vtk unstructured grid
    ///
    static vtkSmartPointer<vtkUnstructuredGrid> convertHexahedralMeshToVtkUnstructuredGrid(std::shared_ptr<HexahedralMesh> imstkMesh);

    ///
    /// \brief
    ///
    static std::shared_ptr<VolumetricMesh> convertVtkUnstructuredGridToVolumetricMesh(vtkUnstructuredGrid* vtkMesh);

    ///
    /// \brief
    ///
    static void copyVerticesFromVtk(vtkPoints* points, StdVectorOfVec3d& vertices);

    ///
    /// \brief Copies vertices from imstk structure to VTK one
    ///
    static void copyVerticesToVtk(const StdVectorOfVec3d& vertices, vtkPoints* points);

    ///
    /// \brief Copies cells of the given dimension from imstk structure to VTK one
    ///
    template<size_t dim>
    static void copyCellsToVtk(const std::vector<std::array<size_t, dim>>& cells, vtkCellArray* vtkCells);

    ///
    /// \brief
    ///
    template<size_t dim>
    static void copyCellsFromVtk(vtkCellArray* vtkCells, std::vector<std::array<size_t, dim>>& cells);

    ///
    /// \brief
    ///
    static void copyPointDataFromVtk(vtkPointData* pointData, std::map<std::string, StdVectorOfVectorf>& dataMap);

public:
    ///
    /// \brief Appends two surface meshes
    ///
    static std::shared_ptr<SurfaceMesh> appendSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh1, std::shared_ptr<SurfaceMesh> surfaceMesh2);

    ///
    /// \brief Converts an imstk SurfaceMesh to a LineMesh, removing duplicate edges. Cell indices not preserved
    ///
    static std::shared_ptr<LineMesh> surfaceMeshToLineMesh(std::shared_ptr<SurfaceMesh> surfaceMesh);

    ///
    /// \brief Smooths a SurfaceMesh using laplacian smoothening
    ///
    static std::shared_ptr<SurfaceMesh> smoothSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
        int numberOfIterations = 20, double relaxationFactor = 0.01,
        double convergence = 0.0, double featureAngle = 45.0,
        double edgeAngle = 15.0, bool featureEdgeSmoothing = false,
        bool boundarySmoothing = true);

    ///
    /// \brief Subidivdes a SurfaceMesh using linear subdivision
    ///
    static std::shared_ptr<SurfaceMesh> linearSubdivideSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh, int numberOfSubdivisions);

    ///
    /// \brief Subidivides a SurfaceMesh using loop subdivision
    ///
    static std::shared_ptr<SurfaceMesh> loopSubdivideSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh, int numberOfSubdivisions);
};
}
