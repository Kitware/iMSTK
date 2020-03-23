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

#include "imstkGeometryUtilities.h"
#include "imstkHexahedralMesh.h"
#include "imstkLineMesh.h"
#include "imstkTetrahedralMesh.h"
#include <vtkAppendPolyData.h>
#include <vtkExtractEdges.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkPointData.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>

#ifndef _MSC_VER
namespace std
{
template<typename T, typename ... Args>
std::unique_ptr<T>
make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
}
#endif

namespace imstk
{
std::unique_ptr<SurfaceMesh>
GeometryUtils::convertVtkPolyDataToSurfaceMesh(vtkSmartPointer<vtkPolyData> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkPolyData provided is not valid!";

    StdVectorOfVec3d vertices;
    copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    std::vector<SurfaceMesh::TriangleArray> triangles;
    copyCellsFromVtk<3>(vtkMesh->GetPolys(), triangles);

    auto mesh = std::make_unique<SurfaceMesh>();
    mesh->initialize(vertices, triangles, true);

    // Point Data
    std::map<std::string, StdVectorOfVectorf> dataMap;
    copyPointDataFromVtk(vtkMesh->GetPointData(), dataMap);
    if (!dataMap.empty())
    {
        mesh->setPointDataMap(dataMap);
    }

    // Active Texture
    if (auto pointData = vtkMesh->GetPointData())
    {
        if (auto tcoords = pointData->GetTCoords())
        {
            mesh->setDefaultTCoords(tcoords->GetName());
        }
    }

    return mesh;
}

std::unique_ptr<LineMesh>
GeometryUtils::convertVtkPolyDataToLineMesh(const vtkSmartPointer<vtkPolyData> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkPolyData provided is not valid!";

    StdVectorOfVec3d vertices;
    copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    std::vector<LineMesh::LineArray> segments;
    copyCellsFromVtk<2>(vtkMesh->GetPolys(), segments);

    auto mesh = std::make_unique<LineMesh>();
    mesh->initialize(vertices, segments);

    // Point Data
    std::map<std::string, StdVectorOfVectorf> dataMap;
    copyPointDataFromVtk(vtkMesh->GetPointData(), dataMap);
    if (!dataMap.empty())
    {
        mesh->setPointDataMap(dataMap);
    }

    return mesh;
}

vtkSmartPointer<vtkPolyData>
GeometryUtils::convertSurfaceMeshToVtkPolyData(const SurfaceMesh& imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh.getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> polys;
    copyCellsToVtk<3>(imstkMesh.getTrianglesVertices(), polys.Get());

    vtkNew<vtkPolyData> polydata;
    polydata->SetPoints(points);
    polydata->SetPolys(polys);

    return polydata;
}

vtkSmartPointer<vtkPolyData>
GeometryUtils::convertLineMeshToVtkPolyData(const LineMesh& imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh.getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> polys;
    copyCellsToVtk<2>(imstkMesh.getLinesVertices(), polys.Get());

    vtkNew<vtkPolyData> polydata;
    polydata->SetPoints(points);
    polydata->SetPolys(polys);
    return polydata;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::convertTetrahedralMeshToVtkUnstructuredGrid(const TetrahedralMesh& imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh.getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> tetras;
    copyCellsToVtk<4>(imstkMesh.getTetrahedraVertices(), tetras.Get());

    vtkNew<vtkUnstructuredGrid> ug;
    ug->SetPoints(points);
    ug->SetCells(VTK_TETRA, tetras);
    return ug;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::convertHexahedralMeshToVtkUnstructuredGrid(const HexahedralMesh& imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh.getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> bricks;
    copyCellsToVtk<8>(imstkMesh.getHexahedraVertices(), bricks.Get());

    vtkNew<vtkUnstructuredGrid> ug;
    ug->SetPoints(points);
    ug->SetCells(VTK_HEXAHEDRON, bricks);
    return ug;
}

std::unique_ptr<VolumetricMesh>
GeometryUtils::convertVtkUnstructuredGridToVolumetricMesh(const vtkSmartPointer<vtkUnstructuredGrid> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkUnstructuredGrid provided is not valid!";

    StdVectorOfVec3d vertices;
    copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    const int cellType = vtkMesh->GetCellType(vtkMesh->GetNumberOfCells() - 1);
    if (cellType == VTK_TETRA)
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        copyCellsFromVtk<4>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_unique<TetrahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else if (cellType == VTK_HEXAHEDRON)
    {
        std::vector<HexahedralMesh::HexaArray> cells;
        copyCellsFromVtk<8>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_unique<HexahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else
    {
        LOG(FATAL) << "No support for vtkCellType = "
                   << cellType << ".";
        return nullptr;
    }
}

void
GeometryUtils::copyVerticesFromVtk(vtkPoints* const points, StdVectorOfVec3d& vertices)
{
    CHECK(points != nullptr) << "No valid point data provided!";

    vertices.reserve(points->GetNumberOfPoints());
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
        double pos[3];
        points->GetPoint(i, pos);
        vertices.emplace_back(pos[0], pos[1], pos[2]);
    }
}

void
GeometryUtils::copyVerticesToVtk(const StdVectorOfVec3d& vertices, vtkPoints* points)
{
    CHECK(points != nullptr) << "No valid vtkPoints pointer provided!";

    points->SetNumberOfPoints(vertices.size());
    for (size_t i = 0; i < vertices.size(); i++)
    {
        points->SetPoint(i, vertices[i][0], vertices[i][1], vertices[i][2]);
    }
}

template<size_t dim>
void
GeometryUtils::copyCellsToVtk(const std::vector<std::array<size_t, dim>>& cells, vtkCellArray* vtkCells)
{
    CHECK(vtkCells != nullptr) << "No valid vtkCellArray provided!";

    for (size_t i = 0; i < cells.size(); i++)
    {
        vtkCells->InsertNextCell(dim);
        for (size_t k = 0; k < dim; k++)
        {
            vtkCells->InsertCellPoint(cells[i][k]);
        }
    }
}

template<size_t dim>
void
GeometryUtils::copyCellsFromVtk(vtkCellArray* vtkCells, std::vector<std::array<size_t, dim>>& cells)
{
    CHECK(vtkCells != nullptr) << "No cells found!";

    vtkNew<vtkIdList>       vtkCell;
    std::array<size_t, dim> cell;
    cells.reserve(vtkCells->GetNumberOfCells());
    vtkCells->InitTraversal();
    while (vtkCells->GetNextCell(vtkCell))
    {
        if (vtkCell->GetNumberOfIds() != dim)
        {
            continue;
        }
        for (size_t i = 0; i < dim; ++i)
        {
            cell[i] = vtkCell->GetId(i);
        }
        cells.emplace_back(cell);
    }
}

void
GeometryUtils::copyPointDataFromVtk(vtkPointData* const pointData, std::map<std::string, StdVectorOfVectorf>& dataMap)
{
    CHECK(pointData != nullptr) << "No point data provided!";

    for (int i = 0; i < pointData->GetNumberOfArrays(); ++i)
    {
        vtkDataArray*      array       = pointData->GetArray(i);
        std::string        name        = array->GetName();
        int                nbrOfComp   = array->GetNumberOfComponents();
        vtkIdType          nbrOfTuples = array->GetNumberOfTuples();
        StdVectorOfVectorf data;
        for (vtkIdType j = 0; j < nbrOfTuples; ++j)
        {
            double* tupleData = new double[nbrOfComp];
            array->GetTuple(j, tupleData);
            Vectorf tuple(nbrOfComp);
            for (int k = 0; k < nbrOfComp; k++)
            {
                tuple[k] = tupleData[k];
            }
            data.push_back(tuple);
        }
        dataMap[name] = data;
    }
}

std::unique_ptr<SurfaceMesh>
GeometryUtils::combineSurfaceMesh(const SurfaceMesh& surfaceMesh1, const SurfaceMesh& surfaceMesh2)
{
    vtkNew<vtkAppendPolyData> filter;
    filter->AddInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh1));
    filter->AddInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh2));
    filter->Update();

    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

std::unique_ptr<LineMesh>
GeometryUtils::surfaceMeshToLineMesh(const SurfaceMesh& surfaceMesh)
{
    vtkNew<vtkExtractEdges> filter;
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->Update();

    vtkNew<vtkTriangleFilter> triangleFilter;
    triangleFilter->SetInputData(filter->GetOutput());
    triangleFilter->Update();

    return convertVtkPolyDataToLineMesh(triangleFilter->GetOutput());
}

std::unique_ptr<SurfaceMesh>
GeometryUtils::smoothSurfaceMesh(const SurfaceMesh&          surfaceMesh,
                                 const smoothPolydataConfig& c)
{
    vtkNew<vtkSmoothPolyDataFilter> filter;
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->SetNumberOfIterations(c.numberOfIterations);
    filter->SetRelaxationFactor(c.relaxationFactor);
    filter->SetConvergence(c.convergence);
    filter->SetFeatureAngle(c.featureAngle);
    filter->SetEdgeAngle(c.edgeAngle);
    filter->SetFeatureEdgeSmoothing(c.featureEdgeSmoothing);
    filter->SetBoundarySmoothing(c.boundarySmoothing);
    filter->Update();

    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

std::unique_ptr<SurfaceMesh>
GeometryUtils::linearSubdivideSurfaceMesh(const SurfaceMesh& surfaceMesh, const int numSubdivisions)
{
    vtkNew<vtkLinearSubdivisionFilter> filter;
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->SetNumberOfSubdivisions(numSubdivisions);
    filter->Update();

    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

std::unique_ptr<SurfaceMesh>
GeometryUtils::loopSubdivideSurfaceMesh(const SurfaceMesh& surfaceMesh, const int numSubdivisions)
{
    vtkNew<vtkLoopSubdivisionFilter> filter;
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->SetNumberOfSubdivisions(numSubdivisions);
    filter->Update();

    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}
}
