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
#include "imstkTypes.h"
#include "imstkVecDataArray.h"

#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vtkSmartPointer.h>
#include <vtkType.h>

class vtkDataArray;
class vtkCellArray;
class vtkPolyData;
class vtkImageData;
class vtkPointData;
class vtkPoints;
class vtkPointSet;
class vtkUnstructuredGrid;

class vtkDataSetAttributes;

namespace imstk
{
class AbstractDataArray;
class ImageData;
class HexahedralMesh;
class LineMesh;
class PointSet;
class SurfaceMesh;
class TetrahedralMesh;
class VolumetricMesh;

class Cube;
class Sphere;
class Plane;

///
/// \brief Contains a set of free functions for processing geometry
/// also contains a set of conversion and coupling functions for VTK
///
namespace GeometryUtils
{
static std::unordered_map<int, ScalarType> vtkToImstkScalarType =
{
    { VTK_VOID, IMSTK_VOID },
    { VTK_CHAR, IMSTK_CHAR },
    { VTK_UNSIGNED_CHAR, IMSTK_UNSIGNED_CHAR },
    { VTK_SHORT, IMSTK_SHORT },
    { VTK_UNSIGNED_SHORT, IMSTK_UNSIGNED_SHORT },
    { VTK_INT, IMSTK_INT },
    { VTK_UNSIGNED_INT, IMSTK_UNSIGNED_INT },
    { VTK_LONG, IMSTK_LONG },
    { VTK_UNSIGNED_LONG, IMSTK_UNSIGNED_LONG },
    { VTK_FLOAT, IMSTK_FLOAT },
    { VTK_DOUBLE, IMSTK_DOUBLE },
    { VTK_LONG_LONG, IMSTK_LONG_LONG },
    { VTK_UNSIGNED_LONG_LONG, IMSTK_UNSIGNED_LONG_LONG }
};
static std::unordered_map<ScalarType, int> imstkToVtkScalarType =
{
    { IMSTK_VOID, VTK_VOID },
    { IMSTK_CHAR, VTK_CHAR },
    { IMSTK_UNSIGNED_CHAR, VTK_UNSIGNED_CHAR },
    { IMSTK_SHORT, VTK_SHORT },
    { IMSTK_UNSIGNED_SHORT, VTK_UNSIGNED_SHORT },
    { IMSTK_INT, VTK_INT },
    { IMSTK_UNSIGNED_INT, VTK_UNSIGNED_INT },
    { IMSTK_LONG, VTK_LONG },
    { IMSTK_UNSIGNED_LONG, VTK_UNSIGNED_LONG },
    { IMSTK_FLOAT, VTK_FLOAT },
    { IMSTK_DOUBLE, VTK_DOUBLE },
    { IMSTK_LONG_LONG, VTK_LONG_LONG },
    { IMSTK_UNSIGNED_LONG_LONG, VTK_UNSIGNED_LONG_LONG }
};

///
/// \brief Coupling functions, these create vtk data objects that point to our data objects
/// thus no copying is done here.
///
vtkSmartPointer<vtkDataArray> coupleVtkDataArray(std::shared_ptr<AbstractDataArray> imstkArray);
vtkSmartPointer<vtkImageData> coupleVtkImageData(std::shared_ptr<ImageData> imstkImageData);

///
/// \brief Copy functions, these copy to/from vtk data objects
///
vtkSmartPointer<vtkDataArray> copyToVtkDataArray(std::shared_ptr<AbstractDataArray> imstkArray);
std::shared_ptr<AbstractDataArray> copyToDataArray(vtkSmartPointer<vtkDataArray> vtkArray);

std::shared_ptr<ImageData> copyToImageData(vtkSmartPointer<vtkImageData> imageDataVtk);
vtkSmartPointer<vtkImageData> copyToVtkImageData(std::shared_ptr<ImageData> imageData);

///
/// \brief Converts vtk polydata into a imstk point set
///
std::shared_ptr<PointSet> copyToPointSet(vtkSmartPointer<vtkPointSet> vtkMesh);

///
/// \brief Converts vtk polydata into a imstk surface mesh
///
std::shared_ptr<SurfaceMesh> copyToSurfaceMesh(vtkSmartPointer<vtkPolyData> vtkMesh);

///
/// \brief Converts vtk polydata into a imstk surface mesh
///
std::shared_ptr<LineMesh> copyToLineMesh(vtkSmartPointer<vtkPolyData> vtkMesh);

///
/// \brief Get imstk volumetric mesh given vtkUnstructuredGrid as input
///
std::shared_ptr<VolumetricMesh> copyToVolumetricMesh(vtkSmartPointer<vtkUnstructuredGrid> vtkMesh);

///
/// \brief Converts imstk point set into a vtk polydata
///
vtkSmartPointer<vtkPointSet> copyToVtkPointSet(std::shared_ptr<PointSet> imstkMesh);

///
/// \brief Converts imstk line mesh into a vtk polydata
///
vtkSmartPointer<vtkPolyData> copyToVtkPolyData(std::shared_ptr<LineMesh> imstkMesh);

///
/// \brief Converts imstk surface mesh into a vtk polydata
///
vtkSmartPointer<vtkPolyData> copyToVtkPolyData(std::shared_ptr<SurfaceMesh> imstkMesh);

///
/// \brief Converts imstk tetrahedral mesh into a vtk unstructured grid
///
vtkSmartPointer<vtkUnstructuredGrid> copyToVtkUnstructuredGrid(std::shared_ptr<TetrahedralMesh> imstkMesh);

///
/// \brief Converts imstk hexahedral mesh into a vtk unstructured grid
///
vtkSmartPointer<vtkUnstructuredGrid> copyToVtkUnstructuredGrid(std::shared_ptr<HexahedralMesh> imstkMesh);

///
/// \brief Copy from vtk points to a imstk vertices array
///
std::shared_ptr<VecDataArray<double, 3>> copyToVecDataArray(vtkSmartPointer<vtkPoints> points);

///
/// \brief Copies vertices from imstk structure to VTK one
///
vtkSmartPointer<vtkPoints> copyToVtkPoints(std::shared_ptr<VecDataArray<double, 3>> vertices);

///
/// \brief Copies cells of the given dimension from imstk structure to VTK one
///
template<size_t dim>
vtkSmartPointer<vtkCellArray> copyToVtkCellArray(std::shared_ptr<VecDataArray<int, dim>> cells);

///
/// \brief Copy from vtk cells to an imstk index array
///
template<size_t dim>
std::shared_ptr<VecDataArray<int, dim>> copyToVecDataArray(vtkCellArray* vtkCells);

///
/// \brief Copy vtkPointData to a data map
///
void copyToDataMap(vtkDataSetAttributes* pointData, std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& dataMap);

///
/// \brief Copy from a data map to vtkDataAttributes (used for vtkCellData and vtkPointData)
/// warning: Components are lost and DataArray's presented as single component
///
void copyToVtkDataAttributes(vtkDataSetAttributes* pointData, const std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& dataMap);

///
/// \brief Produces SurfaceMesh cube from imstkCube
///
std::shared_ptr<SurfaceMesh> toCubeSurfaceMesh(std::shared_ptr<Cube> cube);

///
/// \brief UV sphere from imstkSphere
///
std::shared_ptr<SurfaceMesh> toUVSphereSurfaceMesh(std::shared_ptr<Sphere> sphere,
                                                   const unsigned int phiDivisions, const unsigned int thetaDivisions);

///
/// \brief Produces SurfaceMesh cube from imstkPlane
///
std::shared_ptr<SurfaceMesh> toQuadSurfaceMesh(std::shared_ptr<Plane> plane);

///
/// \brief Returns the number of open edges, use to tell if manifold (==0)
///
int getOpenEdgeCount(std::shared_ptr<SurfaceMesh> surfMesh);

///
/// \brief Returns if the surface is closed or not
///
inline bool
isClosed(std::shared_ptr<SurfaceMesh> surfMesh)
{
    return getOpenEdgeCount(surfMesh) == 0;
}

///
/// \brief Returns volume estimate of closed SurfaceMesh
///
double getVolume(std::shared_ptr<SurfaceMesh> surfMesh);

///
/// \brief Create a tetrahedral mesh based on a uniform Cartesian mesh
/// \param aabbMin  the small corner of a box
/// \param aabbMax  the large corner of a box
/// \param nx number of elements in the x-direction
/// \param ny number of elements in the y-direction
/// \param nz number of elements in the z-direction
///
/// \note Refer: Dompierre, Julien & Labbé, Paul & Vallet, Marie-Gabrielle & Camarero, Ricardo. (1999).
/// How to Subdivide Pyramids, Prisms, and Hexahedra into Tetrahedra.. 195-204.
std::shared_ptr<TetrahedralMesh> createUniformMesh(const Vec3d& aabbMin, const Vec3d& aabbMax, const int nx, const int ny, const int nz);

///
/// \brief Create a tetrahedral mesh cover
///
std::shared_ptr<TetrahedralMesh> createTetrahedralMeshCover(std::shared_ptr<SurfaceMesh> surfMesh, const int nx, const int ny, int nz);

///
/// \brief Enumeration for reordering method
///
enum class MeshNodeRenumberingStrategy
{
    ReverseCuthillMckee     // Reverse Cuthill-Mckee
};

///
/// \brief Reorder indices in a connectivity to reduce bandwidth
///
/// \param[in] neighbors array of neighbors of each vertex; eg, neighbors[i] is an object containing all neighbors of vertex-i
/// \param[i] method reordering method; see \ref ReorderMethod
///
/// \return the permutation vector that map from new indices to old indices
///
template<typename NeighborContainer>
std::vector<size_t> reorderConnectivity(const std::vector<NeighborContainer>& neighbors, const MeshNodeRenumberingStrategy& method = MeshNodeRenumberingStrategy::ReverseCuthillMckee);

///
/// \brief Reorder using Reverse Cuthill-Mckee
///
/// \param[in] conn element-to-vertex connectivity
/// \param[in] numVerts number of vertices
/// \param[in] method reordering method; see \ref ReorderMethod
///
/// \return the permutation vector that maps from new indices to old indices
///
template<typename ElemConn>
std::vector<size_t> reorderConnectivity(const std::vector<ElemConn>& conn, const size_t numVerts, const MeshNodeRenumberingStrategy& method = MeshNodeRenumberingStrategy::ReverseCuthillMckee);
} // namespace GeometryUtils
} // namespace imstk
