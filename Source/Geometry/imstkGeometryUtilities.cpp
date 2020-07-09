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
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include <vtkAppendPolyData.h>
#include <vtkCleanPolyData.h>
#include <vtkDataSet.h>
#include <vtkPolyData.h>
#include <vtkPointSet.h>
#include <vtkExtractEdges.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkPointData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <vtkFloatArray.h>



namespace imstk
{
std::unique_ptr<PointSet>
GeometryUtils::convertVtkPointSetToPointSet(const vtkSmartPointer<vtkPointSet> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkPolyData provided is not valid!";

    StdVectorOfVec3d vertices;
    copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    auto mesh = std::make_unique<PointSet>();
    mesh->initialize(vertices);

    // Point Data
    std::map<std::string, StdVectorOfVectorf> dataMap;
    copyPointDataFromVtk(vtkMesh->GetPointData(), dataMap);
    if (!dataMap.empty())
    {
        mesh->setPointDataMap(dataMap);
    }

    return mesh;
}

std::unique_ptr<SurfaceMesh>
GeometryUtils::convertVtkPolyDataToSurfaceMesh(const vtkSmartPointer<vtkPolyData> vtkMesh)
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

vtkSmartPointer<vtkPointSet>
GeometryUtils::convertPointSetToVtkPointSet(const std::shared_ptr<PointSet> imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    return polyData;
}

vtkSmartPointer<vtkPolyData>
GeometryUtils::convertLineMeshToVtkPolyData(const std::shared_ptr<LineMesh> imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> polys;
    copyCellsToVtk<2>(imstkMesh->getLinesVertices(), polys.Get());

    auto polydata = vtkSmartPointer<vtkPolyData>::New();;
    polydata->SetPoints(points);
    polydata->SetPolys(polys);
    return polydata;
}

vtkSmartPointer<vtkPolyData>
GeometryUtils::convertSurfaceMeshToVtkPolyData(const std::shared_ptr<SurfaceMesh> imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> polys;
    copyCellsToVtk<3>(imstkMesh->getTrianglesVertices(), polys.Get());

    auto polydata = vtkSmartPointer<vtkPolyData>::New();;
    polydata->SetPoints(points);
    polydata->SetPolys(polys);

    return polydata;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::convertTetrahedralMeshToVtkUnstructuredGrid(const std::shared_ptr<TetrahedralMesh> imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> tetras;
    copyCellsToVtk<4>(imstkMesh->getTetrahedraVertices(), tetras.Get());

    auto ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_TETRA, tetras);

    const std::map<std::string, StdVectorOfVectorf>& pointDataArrays = imstkMesh->getPointDataMap();
    for (auto i : pointDataArrays)
    {
      StdVectorOfVectorf& pointDataArray = i.second;
      if (pointDataArray.size() == 0)
      {
        continue;
      }
      //Assume homogenous vector size
      vtkNew<vtkFloatArray> arrVtk;
      arrVtk->SetName(i.first.c_str());
      arrVtk->SetNumberOfComponents(i.second[0].size());
      arrVtk->SetNumberOfTuples(pointDataArray.size());
      for (int j = 0; j < pointDataArray.size(); j++)
      {
        arrVtk->SetTuple(j, pointDataArray[j].data());
      }
      ug->GetPointData()->AddArray(arrVtk);
    }

    return ug;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::convertHexahedralMeshToVtkUnstructuredGrid(const std::shared_ptr<HexahedralMesh> imstkMesh)
{
    vtkNew<vtkPoints> points;
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkNew<vtkCellArray> bricks;
    copyCellsToVtk<8>(imstkMesh->getHexahedraVertices(), bricks.Get());

    auto ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_HEXAHEDRON, bricks);
    return ug;
}

std::unique_ptr<VolumetricMesh>
GeometryUtils::convertVtkUnstructuredGridToVolumetricMesh(vtkSmartPointer<vtkUnstructuredGrid> vtkMesh)
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
GeometryUtils::combineSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh1, std::shared_ptr<SurfaceMesh> surfaceMesh2)
{
    vtkNew<vtkAppendPolyData> filter;
    filter->AddInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh1));
    filter->AddInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh2));
    filter->Update();

    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

std::unique_ptr<LineMesh>
GeometryUtils::surfaceMeshToLineMesh(std::shared_ptr<SurfaceMesh> surfaceMesh)
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
GeometryUtils::cleanSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh)
{
    vtkNew<vtkCleanPolyData> filter;
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->Update();
    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

std::unique_ptr<PointSet>
GeometryUtils::getEnclosedPoints(std::shared_ptr<SurfaceMesh> surfaceMesh, std::shared_ptr<PointSet> pointSet, bool insideOut)
{
    vtkNew<vtkSelectEnclosedPoints> filter;
    filter->SetInputData(convertPointSetToVtkPointSet(pointSet));
    filter->SetSurfaceData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->SetTolerance(0.0);
    filter->SetInsideOut(insideOut);
    filter->Update();
    vtkSmartPointer<vtkPointSet> vtkResults = vtkPointSet::SafeDownCast(filter->GetOutput());

    StdVectorOfVec3d points;
    points.reserve(vtkResults->GetNumberOfPoints());
    for (vtkIdType i = 0; i < vtkResults->GetNumberOfPoints(); i++)
    {
        if (filter->IsInside(i))
        {
            double pt[3];
            vtkResults->GetPoint(i, pt);
            points.push_back(Vec3d(pt[0], pt[1], pt[2]));
        }
    }
    points.shrink_to_fit();

    std::unique_ptr<PointSet> results = std::make_unique<PointSet>();
    results->setInitialVertexPositions(points);
    results->setVertexPositions(points);
    return results;
}

void
GeometryUtils::testEnclosedPoints(std::vector<bool>& results, std::shared_ptr<SurfaceMesh> surfaceMesh, std::shared_ptr<PointSet> pointSet, const bool insideOut)
{
    vtkNew<vtkSelectEnclosedPoints> filter;
    filter->SetInputData(convertPointSetToVtkPointSet(pointSet));
    filter->SetSurfaceData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->SetTolerance(0.0);
    filter->SetInsideOut(insideOut);
    filter->Update();
    vtkUnsignedCharArray* arr = vtkUnsignedCharArray::SafeDownCast(filter->GetOutput()->GetPointData()->GetArray("SelectedPoints"));

    results.resize(arr->GetNumberOfValues());
    for (vtkIdType i = 0; i < arr->GetNumberOfValues(); i++)
    {
        results[i] = (arr->GetValue(i) ? true : false);
    }
}

std::unique_ptr<SurfaceMesh>
GeometryUtils::smoothSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
                                 const smoothPolydataConfig&  c)
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
GeometryUtils::linearSubdivideSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh, const int numSubdivisions)
{
    vtkNew<vtkLinearSubdivisionFilter> filter;
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->SetNumberOfSubdivisions(numSubdivisions);
    filter->Update();

    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

std::unique_ptr<SurfaceMesh>
GeometryUtils::loopSubdivideSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh, const int numSubdivisions)
{
    vtkNew<vtkLoopSubdivisionFilter> filter;
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh));
    filter->SetNumberOfSubdivisions(numSubdivisions);
    filter->Update();

    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

namespace // anonymous namespace
{
///
/// \brief Build the vertex-to-vertex connectivity
///
/// \param[in] conn element-to-vertex connectivity
/// \param[in] numVerts number of vertices
/// \param[out] vertToVert the vertex-to-vertex connectivity on return
///
template<typename ElemConn>
static void
buildVertexToVertexConnectivity(const std::vector<ElemConn>&             conn,
                                const size_t                             numVerts,
                                std::vector<std::unordered_set<size_t>>& vertToVert)
{
    // constexpr size_t numVertPerElem = ElemConn::size();
    std::vector<size_t> vertToElemPtr(numVerts + 1, 0);
    std::vector<size_t> vertToElem;

    // find the number of adjacent elements for each vertex
    for (const auto& vertices : conn)
    {
        for (auto vid : vertices)
        {
            vertToElemPtr[vid + 1] += 1;
        }
    }

    // accumulate pointer
    for (size_t i = 0; i < numVerts; ++i)
    {
        vertToElemPtr[i + 1] += vertToElemPtr[i];
    }

    // track the front position for each vertex in \p vertToElem
    auto pos = vertToElemPtr;
    // the total number of element neighbors of all vertices, ie, size of \p vertToElem
    const size_t totNum = vertToElemPtr.back();

    vertToElem.resize(totNum);

    for (size_t eid = 0; eid < conn.size(); ++eid)
    {
        for (auto vid : conn[eid])
        {
            vertToElem[pos[vid]] = eid;
            ++pos[vid];
        }
    }

    // connectivity of vertex-to-vertex
    vertToVert.resize(numVerts);
    auto getVertexNbrs = [&vertToElem, &vertToElemPtr, &conn, &vertToVert](const size_t i)
                         {
                             const auto ptr0 = vertToElemPtr[i];
                             const auto ptr1 = vertToElemPtr[i + 1];

                             for (auto ptr = ptr0; ptr < ptr1; ++ptr)
                             {
                                 for (auto vid : conn[vertToElem[ptr]])
                                 {
                                     // vertex-i itself is also included.
                                     vertToVert[i].insert(vid);
                                 }
                             }
                         };

    // for (size_t i = 0; i < numVerts; ++i)
    // {
    //     getVertexNbrs(i);
    // }
    ParallelUtils::parallelFor(numVerts, getVertexNbrs);
}

///
/// \brief Reverse Cuthill-Mckee (RCM) based reording to reduce bandwidth
///
/// \param[in] neighbors array of neighbors of each vertex; eg, neighbors[i] is an object containing all neighbors of vertex-i
/// \return the permutation vector that map from new indices to old indices
///
/// \cite https://en.wikipedia.org/wiki/Cuthill%E2%80%93McKee_algorithm
///
template<typename NeighborContainer>
static std::vector<size_t>
RCM(const std::vector<NeighborContainer>& neighbors)
{
    const size_t invalid = std::numeric_limits<size_t>::max();

    // is greater in terms of degrees
    auto isGreater = [&neighbors](const size_t i, const size_t j) {
                         return neighbors[i].size() > neighbors[j].size() ? true : false;
                     };

    const size_t numVerts = neighbors.size();

    std::vector<size_t> P(numVerts);
    for (size_t i = 0; i < numVerts; ++i)
    {
        P[i] = i;
    }
    std::sort(P.begin(), P.end(), isGreater);

    // \todo an alternative is to use std::set for P
    // std::set<size_t, isGreater> P;
    // for (size_t i=0; i<numVerts; ++i)
    // {
    //     P.insert(i);
    // }

    std::vector<size_t> R(numVerts, invalid);      // permutation
    std::queue<size_t>  Q;                         // queue
    std::vector<bool>   isInP(numVerts, true);     // if a vertex is still in P
    size_t              pos = 0;                   // track how many vertices are put into R

    ///
    /// \brief Move a vertex into R, and enque its neighbors into Q in ascending order.
    /// \param vid index of vertex to be moved into R
    ///
    auto moveVertexIntoRAndItsNeighborsIntoQ = [&neighbors, &isInP, &pos, &R, &Q](const size_t vid)
                                               {
                                                   R[pos] = vid;
                                                   ++pos;
                                                   isInP[vid] = false;

                                                   // Put the unordered neighbors into Q, in ascending order.
                                                   // first find unordered neighbors
                                                   std::set<size_t> unorderedNbrs;
                                                   for (auto nbr : neighbors[vid])
                                                   {
                                                       if (isInP[nbr])
                                                       {
                                                           unorderedNbrs.insert(nbr);
                                                       }
                                                   }

                                                   for (auto nbr : unorderedNbrs)
                                                   {
                                                       Q.push(nbr);
                                                       isInP[nbr] = false;
                                                   }
                                               };

    size_t pCur = 0;

    // main loop
    while (true)
    {
        size_t parent = invalid;
        for (size_t vid = pCur; vid < isInP.size(); ++vid)
        {
            if (isInP[vid])
            {
                isInP[vid] = false;
                pCur       = vid;
                parent     = vid;
                break;
            }
        }
        if (parent == invalid)
        {
            break;
        }

        moveVertexIntoRAndItsNeighborsIntoQ(parent);

        while (!Q.empty())
        {
            parent = Q.front();
            Q.pop();
            moveVertexIntoRAndItsNeighborsIntoQ(parent);
        }

        // here we have empty Q
    }

    CHECK(pos == numVerts) << "RCM ordering: we should never get here";

    std::reverse(R.begin(), R.end());
    return R;
}

///
/// \brief Reorder using Reverse Cuthill-Mckee algorithm
//
/// \param conn element-to-vertex connectivity
/// \param numVerts number of vertices
///
/// \return the permutation vector that maps from new indices to old indices
///
template<typename ElemConn>
static std::vector<size_t>
RCM(const std::vector<ElemConn>& conn, const size_t numVerts)
{
    // connectivity of vertex-to-vertex
    std::vector<std::unordered_set<size_t>> vertToVert;
    buildVertexToVertexConnectivity(conn, numVerts, vertToVert);
    return RCM(vertToVert);
}

///
/// \brief Given a set of points mark them as inside (true) and outside
/// \param surfaceMesh a \ref SurfaceMesh
/// \param coords a set of points to be tested
///
/// \note this function cannot be const because PointSet::computeBoundingBox, called inside, is not.
///
void
markPointsInsideAndOut(std::vector<bool>&      isInside,
                       const SurfaceMesh&      surfaceMesh,
                       const StdVectorOfVec3d& coords)
{
    isInside.clear();
    isInside.resize(coords.size(), false);

    Vec3d aabbMin, aabbMax;
    surfaceMesh.computeBoundingBox(aabbMin, aabbMax, 1.);

    auto genRandomDirection = [](Vec3d& direction)
                              {
                                  for (int i = 0; i < 3; ++i)
                                  {
                                      direction[i] = rand();
                                  }
                                  double mag = direction.norm();

                                  for (int i = 0; i < 3; ++i)
                                  {
                                      direction[i] /= mag;
                                  }
                                  return;
                              };

    auto triangleRayIntersection = [](const Vec3d& xyz,
                                      const Vec3d& triVert0,
                                      const Vec3d& triVert1,
                                      const Vec3d& triVert2,
                                      const Vec3d& direction)
                                   {
                                       // const double eps = 1e-15;
                                       constexpr const double eps   = std::numeric_limits<double>::epsilon();
                                       Vec3d                  edge0 = triVert1 - triVert0;
                                       Vec3d                  edge1 = triVert2 - triVert0;
                                       Vec3d                  pvec  = direction.cross(edge1);
                                       double                 det   = edge0.dot(pvec);

                                       if (det > -eps && det < eps)
                                       {
                                           return false;
                                       }
                                       double inv_det = 1.0 / det;
                                       Vec3d  tvec    = xyz - triVert0;
                                       double u       = tvec.dot(pvec) * inv_det;
                                       if (u < 0.0 || u > 1.0)
                                       {
                                           return false;
                                       }
                                       Vec3d  qvec = tvec.cross(edge0);
                                       double v    = direction.dot(qvec) * inv_det;
                                       if (v < 0.0 || u + v > 1.0)
                                       {
                                           return false;
                                       }

                                       double t = edge1.dot(qvec) * inv_det;
                                       if (t > 0.0)
                                       {
                                           return true;
                                       }
                                       else
                                       {
                                           return false;
                                       }
                                   };

    std::vector<Vec3d> bBoxMin;
    std::vector<Vec3d> bBoxMax;

    bBoxMin.resize(surfaceMesh.getNumTriangles());
    bBoxMax.resize(surfaceMesh.getNumTriangles());

    for (size_t idx = 0; idx < surfaceMesh.getNumTriangles(); ++idx)
    {
        const auto& verts = surfaceMesh.getTrianglesVertices().at(idx);
        const auto& xyz0  = surfaceMesh.getVertexPosition(verts[0]);
        const auto& xyz1  = surfaceMesh.getVertexPosition(verts[1]);
        const auto& xyz2  = surfaceMesh.getVertexPosition(verts[2]);
        // const auto& xyz0  = m_vertexPositions[verts[0]];
        // const auto& xyz1  = m_vertexPositions[verts[1]];
        // const auto& xyz2  = m_vertexPositions[verts[2]];

        bBoxMin[idx][0] = xyz0[0];
        bBoxMin[idx][1] = xyz0[1];
        bBoxMin[idx][2] = xyz0[2];
        bBoxMax[idx][0] = xyz0[0];
        bBoxMax[idx][1] = xyz0[1];
        bBoxMax[idx][2] = xyz0[2];

        bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz1[0]);
        bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz1[1]);
        bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz1[2]);
        bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz2[0]);
        bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz2[1]);
        bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz2[2]);

        bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz1[0]);
        bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz1[1]);
        bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz1[2]);
        bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz2[0]);
        bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz2[1]);
        bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz2[2]);
    }

    auto rayTracingFunc = [&coords,
                           &aabbMin,
                           &aabbMax,
                           &bBoxMin,
                           &bBoxMax,
                           &isInside,
                           &triangleRayIntersection,
                           &genRandomDirection,
                           &surfaceMesh](const size_t i)
                          {
                              bool outBox = coords[i][0] < aabbMin[0] || coords[i][0] > aabbMax[0]
                                            || coords[i][1] < aabbMin[1] || coords[i][1] > aabbMax[1]
                                            || coords[i][2] < aabbMin[2] || coords[i][2] > aabbMax[2];
                              if (outBox)
                              {
                                  return;
                              }

                              /// \todo generate a random direction?
                              const Vec3d direction = { 0.0, 0.0, 1.0 };
                              // Vec3d direction;
                              // genRandomDirection(direction);
                              // std::cout << direction << std::endl;
                              int         numIntersections = 0;
                              const auto& xyz      = surfaceMesh.getVertexPositions();
                              const auto& triVerts = surfaceMesh.getTrianglesVertices();

                              for (size_t j = 0; j < surfaceMesh.getNumTriangles(); ++j)
                              {
                                  const auto& verts = triVerts[j];

                                  // consider directed ray
                                  if (coords[i][2] > bBoxMax[j][2])
                                  {
                                      continue;
                                  }

                                  if (coords[i][0] > bBoxMax[j][0])
                                  {
                                      continue;
                                  }
                                  if (coords[i][0] < bBoxMin[j][0])
                                  {
                                      continue;
                                  }
                                  if (coords[i][1] > bBoxMax[j][1])
                                  {
                                      continue;
                                  }
                                  if (coords[i][1] < bBoxMin[j][1])
                                  {
                                      continue;
                                  }

                                  auto intersected = triangleRayIntersection(coords[i],
                                xyz[verts[0]],
                                xyz[verts[1]],
                                xyz[verts[2]],
                                direction);
                                  if (intersected)
                                  {
                                      ++numIntersections;
                                  }
                              }

                              if (numIntersections % 2 == 1)
                              {
                                  isInside[i] = true;
                              }

                              return;
                          };

    // for (size_t i = 0; i < coords.size(); ++i)
    // {
    //     rayTracingFunc(i);
    // }

    ParallelUtils::parallelFor(coords.size(), rayTracingFunc);
}

///
/// \brief Given a set of uniformly spaced points, mark them as inside (true) and outside.
/// It makes uses of ray-tracing but skips points based on the nearest distance between current point and the surface.
///
/// \param surfaceMesh a \ref SurfaceMesh
/// \param coords a set of points to be tested
/// \param nx number of points in x-direction
/// \param ny number of points in y-direction
/// \param nz number of points in z-direction
///
/// \note this function cannot be const because PointSet::computeBoundingBox, called inside, is not.
///
void
markPointsInsideAndOut(std::vector<bool>&      isInside,
                       const SurfaceMesh&      surfaceMesh,
                       const StdVectorOfVec3d& coords,
                       const size_t            nx,
                       const size_t            ny,
                       const size_t            nz)
{
    isInside.clear();
    isInside.resize(coords.size(), false);

    Vec3d aabbMin, aabbMax;
    surfaceMesh.computeBoundingBox(aabbMin, aabbMax, 1.);
    // space between two adjacent points
    const Vec3d h = { coords[1][0] - coords[0][0], coords[nx][1] - coords[0][1], coords[nx * ny][2] - coords[0][2] };

    /// \brief if a ray intersects with a triangle.
    /// \param xyz starting point of ray
    /// \param triVert0 triVert0, triVert1, triVert2 are 3 vertices of the triangle
    /// \param direction direction of the ray
    /// \param[out] distance on return it is the distance of the point and the triangle
    auto triangleRayIntersection = [](const Vec3d& xyz, const Vec3d& triVert0, const Vec3d& triVert1, const Vec3d& triVert2, const Vec3d& direction, double& distance)
                                   {
                                       const double eps   = std::numeric_limits<double>::epsilon();
                                       const Vec3d  edge0 = triVert1 - triVert0;
                                       const Vec3d  edge1 = triVert2 - triVert0;
                                       const Vec3d  pvec  = direction.cross(edge1);
                                       const double det   = edge0.dot(pvec);

                                       if (det > -eps && det < eps)
                                       {
                                           return false;
                                       }
                                       const double inv_det = 1.0 / det;
                                       const Vec3d  tvec    = xyz - triVert0;
                                       const double u       = tvec.dot(pvec) * inv_det;
                                       if (u < 0.0 || u > 1.0)
                                       {
                                           return false;
                                       }
                                       const Vec3d  qvec = tvec.cross(edge0);
                                       const double v    = direction.dot(qvec) * inv_det;
                                       if (v < 0.0 || u + v > 1.0)
                                       {
                                           return false;
                                       }

                                       distance = edge1.dot(qvec) * inv_det;
                                       if (distance > 0.0)
                                       {
                                           return true;
                                       }
                                       else
                                       {
                                           return false;
                                       }
                                   };

    std::vector<Vec3d> bBoxMin;
    std::vector<Vec3d> bBoxMax;

    bBoxMin.resize(surfaceMesh.getNumTriangles());
    bBoxMax.resize(surfaceMesh.getNumTriangles());

    /// \brief find the bounding boxes of each surface triangle
    auto findBoundingBox = [&](const size_t idx)
                           {
                               const auto& verts   = surfaceMesh.getTrianglesVertices().at(idx);
                               const auto& vertXyz = surfaceMesh.getVertexPositions();
                               const auto& xyz0    = vertXyz[verts[0]];
                               const auto& xyz1    = vertXyz[verts[1]];
                               const auto& xyz2    = vertXyz[verts[2]];

                               bBoxMin[idx][0] = xyz0[0];
                               bBoxMin[idx][1] = xyz0[1];
                               bBoxMin[idx][2] = xyz0[2];
                               bBoxMax[idx][0] = xyz0[0];
                               bBoxMax[idx][1] = xyz0[1];
                               bBoxMax[idx][2] = xyz0[2];

                               bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz1[0]);
                               bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz1[1]);
                               bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz1[2]);
                               bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz2[0]);
                               bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz2[1]);
                               bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz2[2]);

                               bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz1[0]);
                               bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz1[1]);
                               bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz1[2]);
                               bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz2[0]);
                               bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz2[1]);
                               bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz2[2]);
                           };

    ParallelUtils::parallelFor(surfaceMesh.getNumTriangles(), findBoundingBox);

    // ray tracing for all points in the x-axis. These points are those start with indices (0,j,k)
    // and jk = j + k*ny
    auto rayTracingLine = [&](const size_t jk)
                          {
                              size_t idx0   = jk * nx;
                              bool   outBox = coords[idx0][0] < aabbMin[0] || coords[idx0][0] > aabbMax[0]
                                              || coords[idx0][1] < aabbMin[1] || coords[idx0][1] > aabbMax[1]
                                              || coords[idx0][2] < aabbMin[2] || coords[idx0][2] > aabbMax[2];
                              if (outBox)
                              {
                                  return;
                              }

                              const Vec3d direction = { 1.0, 0.0, 0.0 };
                              const auto& xyz       = surfaceMesh.getVertexPositions();
                              const auto& triVerts  = surfaceMesh.getTrianglesVertices();

                              size_t i = 0;
                              while (i < nx)
                              {
                                  size_t idx = idx0 + i;
                                  int    numIntersections = 0;
                                  double dist    = 0.0;
                                  double distMin = h[0] * (nz + 1);

                                  for (size_t j = 0; j < surfaceMesh.getNumTriangles(); ++j)
                                  {
                                      const auto& verts = triVerts[j];

                                      // consider directed ray
                                      if (coords[idx][0] > bBoxMax[j][0])
                                      {
                                          continue;
                                      }

                                      if (coords[idx][1] > bBoxMax[j][1])
                                      {
                                          continue;
                                      }
                                      if (coords[idx][1] < bBoxMin[j][1])
                                      {
                                          continue;
                                      }
                                      if (coords[idx][2] > bBoxMax[j][2])
                                      {
                                          continue;
                                      }
                                      if (coords[idx][2] < bBoxMin[j][2])
                                      {
                                          continue;
                                      }

                                      auto intersected = triangleRayIntersection(coords[idx],
                                        xyz[verts[0]],
                                        xyz[verts[1]],
                                        xyz[verts[2]],
                                        direction,
                                        dist);
                                      if (intersected)
                                      {
                                          ++numIntersections;
                                          distMin = std::min(dist, distMin);
                                      }
                                  }

                                  // core of the algorithm: points between the current one and iEnd share the same label so we can skip them.
                                  size_t iEnd = i + static_cast<size_t>(distMin / h[0]) + 1;
                                  iEnd = std::min(iEnd, nx);

                                  if (numIntersections % 2 == 1)
                                  {
                                      for (size_t ii = idx; ii < idx0 + iEnd; ++ii)
                                      {
                                          isInside[ii] = true;
                                      }
                                  }

                                  i = iEnd;
                              }
                          };

    ParallelUtils::parallelFor(ny * nz, rayTracingLine);
}
} // anonymous namespace

std::shared_ptr<TetrahedralMesh>
GeometryUtils::createUniformMesh(const Vec3d& aabbMin,
                                 const Vec3d& aabbMax,
                                 const size_t nx,
                                 const size_t ny,
                                 const size_t nz)
{
    const Vec3d h = { (aabbMax[0] - aabbMin[0]) / nx,
                      (aabbMax[1] - aabbMin[1]) / ny,
                      (aabbMax[2] - aabbMin[2]) / nz };
    LOG_IF(FATAL, (h[0] <= 0.0 || h[1] <= 0.0 || h[2] <= 0.0)) << "Invalid bounding box";

    const size_t numVertices = (nx + 1) * (ny + 1) * (nz + 1);

    // std::vector<Vec3d> coords;
    StdVectorOfVec3d coords;
    coords.resize(numVertices);
    size_t cnt = 0;

    for (size_t k = 0; k < nz + 1; ++k)
    {
        double z = aabbMin[2] + k * h[2];
        for (size_t j = 0; j < ny + 1; ++j)
        {
            double y = aabbMin[1] + j * h[1];
            for (size_t i = 0; i < nx + 1; ++i)
            {
                double x = aabbMin[0] + i * h[0];
                coords[cnt] = { x, y, z };
                ++cnt;
            }
        }
    }

    const size_t numDiv  = 6;
    const size_t numTets = numDiv * nx * ny * nz;

    std::vector<TetrahedralMesh::TetraArray> vertices;
    vertices.resize(numTets);
    cnt = 0;
    std::vector<size_t> indx(8);

    for (size_t k = 0; k < nz; ++k)
    {
        for (size_t j = 0; j < ny; ++j)
        {
            for (size_t i = 0; i < nx; ++i)
            {
                indx[3] = i + j * (nx + 1) + k * (nx + 1) * (ny + 1);
                indx[2] = indx[3] + 1;
                indx[0] = indx[3] + nx + 1;
                indx[1] = indx[0] + 1;
                indx[4] = indx[0] + (nx + 1) * (ny + 1);
                indx[5] = indx[1] + (nx + 1) * (ny + 1);
                indx[6] = indx[2] + (nx + 1) * (ny + 1);
                indx[7] = indx[3] + (nx + 1) * (ny + 1);
                vertices[cnt + 0] = { indx[0], indx[2], indx[3], indx[6] };
                vertices[cnt + 1] = { indx[0], indx[3], indx[7], indx[6] };
                vertices[cnt + 2] = { indx[0], indx[7], indx[4], indx[6] };
                vertices[cnt + 3] = { indx[0], indx[5], indx[6], indx[4] };
                vertices[cnt + 4] = { indx[1], indx[5], indx[6], indx[0] };
                vertices[cnt + 5] = { indx[1], indx[6], indx[2], indx[0] };
                cnt += numDiv;
            }
        }
    }

    auto mesh = std::make_shared<TetrahedralMesh>();
    mesh->initialize(coords, vertices);
    return mesh;
}

std::shared_ptr<TetrahedralMesh>
GeometryUtils::createTetrahedralMeshCover(std::shared_ptr<SurfaceMesh> surfMesh,
                                          const size_t                 nx,
                                          const size_t                 ny,
                                          const size_t                 nz)
{
    Vec3d aabbMin, aabbMax;

    // create a background mesh
    surfMesh->computeBoundingBox(aabbMin, aabbMax, 1. /*percentage padding*/);
    auto uniformMesh = createUniformMesh(aabbMin, aabbMax, nx, ny, nz);

    // ray-tracing
    const auto&       coords = uniformMesh->getVertexPositions();
    std::vector<bool> insideSurfMesh;
    markPointsInsideAndOut(insideSurfMesh, *surfMesh.get(), coords, nx + 1, ny + 1, nz + 1);

    // label elements
    std::vector<bool> validTet(uniformMesh->getNumTetrahedra(), false);
    std::vector<bool> validVtx(uniformMesh->getNumVertices(), false);

    // TetrahedralMesh::WeightsArray weights = {0.0, 0.0, 0.0, 0.0};
    const Vec3d h = { (aabbMax[0] - aabbMin[0]) / nx,
                      (aabbMax[1] - aabbMin[1]) / ny,
                      (aabbMax[2] - aabbMin[2]) / nz };

    // a customized approach to find the enclosing tet for each surface points
    /// \todo can be parallelized by make NUM_THREADS copies of validTet, or use atomic op on validTet
    auto labelEnclosingTet = [&aabbMin, &h, nx, ny, nz, &uniformMesh, &validTet](const Vec3d& xyz)
                             {
                                 const size_t idX   = (xyz[0] - aabbMin[0]) / h[0];
                                 const size_t idY   = (xyz[1] - aabbMin[1]) / h[1];
                                 const size_t idZ   = (xyz[2] - aabbMin[2]) / h[2];
                                 const size_t hexId = idX + idY * nx + idZ * nx * ny;

                                 // the index range of tets inside the enclosing hex
                                 const int    numDiv = 6;
                                 const size_t tetId0 = numDiv * hexId;
                                 const size_t tetId1 = tetId0 + numDiv;

                                 static TetrahedralMesh::WeightsArray weights = { 0.0, 0.0, 0.0, 0.0 };

                                 // loop over the tets to find the enclosing tets
                                 for (size_t id = tetId0; id < tetId1; ++id)
                                 {
                                     if (validTet[id])
                                     {
                                         continue;
                                     }
                                     uniformMesh->computeBarycentricWeights(id, xyz, weights);

                                     if ((weights[0] >= 0) && (weights[1] >= 0) && (weights[2] >= 0) && (weights[3] >= 0))
                                     {
                                         validTet[id] = true;
                                         break;
                                     }
                                 }
                             };

    auto labelEnclosingTetOfVertices = [&surfMesh, &uniformMesh, &aabbMin, &h, nx, ny, nz, &labelEnclosingTet, &validTet](const size_t i)
                                       {
                                           const auto& xyz = surfMesh->getVertexPosition(i);
                                           labelEnclosingTet(xyz);
                                       };

    for (size_t i = 0; i < validTet.size(); ++i)
    {
        const auto& verts = uniformMesh->getTetrahedronVertices(i);
        if (insideSurfMesh[verts[0]]
            || insideSurfMesh[verts[1]]
            || insideSurfMesh[verts[2]]
            || insideSurfMesh[verts[3]])
        {
            validTet[i] = true;
        }
    }

    // find the enclosing tets of a group of points on a surface triangle
    auto labelEnclosingTetOfInteriorPnt = [&surfMesh, &labelEnclosingTet](const size_t fid)
                                          {
                                              auto               verts = surfMesh->getTrianglesVertices()[fid];
                                              const auto&        vtx0  = surfMesh->getVertexPosition(verts[0]);
                                              const auto&        vtx1  = surfMesh->getVertexPosition(verts[1]);
                                              const auto&        vtx2  = surfMesh->getVertexPosition(verts[2]);
                                              std::vector<Vec3d> pnts(12);

                                              pnts[0]  = 0.75 * vtx0 + 0.25 * vtx1;
                                              pnts[1]  = 0.50 * vtx0 + 0.50 * vtx1;
                                              pnts[2]  = 0.25 * vtx0 + 0.75 * vtx1;
                                              pnts[3]  = 0.75 * vtx1 + 0.25 * vtx2;
                                              pnts[4]  = 0.50 * vtx1 + 0.50 * vtx2;
                                              pnts[5]  = 0.25 * vtx1 + 0.75 * vtx2;
                                              pnts[6]  = 0.75 * vtx2 + 0.25 * vtx0;
                                              pnts[7]  = 0.50 * vtx2 + 0.50 * vtx0;
                                              pnts[8]  = 0.25 * vtx2 + 0.75 * vtx0;
                                              pnts[9]  = 2.0 / 3.0 * pnts[0] + 1.0 / 3.0 * pnts[5];
                                              pnts[10] = 0.5 * (pnts[1] + pnts[4]);
                                              pnts[11] = 0.5 * (pnts[4] + pnts[7]);

                                              for (size_t i = 0; i < pnts.size(); ++i)
                                              {
                                                  labelEnclosingTet(pnts[i]);
                                              }
                                          };

    // enclose all vertices
    for (size_t i = 0; i < surfMesh->getNumVertices(); ++i)
    {
        labelEnclosingTetOfVertices(i);
    }

    // enclose some interior points on triangles
    for (size_t i = 0; i < surfMesh->getNumTriangles(); ++i)
    {
        labelEnclosingTetOfInteriorPnt(i);
    }

    size_t numElems = 0;
    for (size_t i = 0; i < validTet.size(); ++i)
    {
        const auto& verts = uniformMesh->getTetrahedronVertices(i);
        if (validTet[i])
        {
            validVtx[verts[0]] = true;
            validVtx[verts[1]] = true;
            validVtx[verts[2]] = true;
            validVtx[verts[3]] = true;
            ++numElems;
        }
    }

    // discard useless vertices, and build old-to-new index map
    size_t numVerts = 0;
    for (auto b : validVtx)
    {
        if (b)
        {
            ++numVerts;
        }
    }

    StdVectorOfVec3d    newCoords(numVerts);
    std::vector<size_t> oldToNew(coords.size(), std::numeric_limits<size_t>::max());

    size_t cnt = 0;
    for (size_t i = 0; i < validVtx.size(); ++i)
    {
        if (validVtx[i])
        {
            newCoords[cnt] = coords[i];
            oldToNew[i]    = cnt;
            ++cnt;
        }
    }

    // update tet-to-vert connectivity
    std::vector<TetrahedralMesh::TetraArray> newIndices(numElems);
    cnt = 0;
    for (size_t i = 0; i < uniformMesh->getNumTetrahedra(); ++i)
    {
        if (validTet[i])
        {
            const auto oldIndices = uniformMesh->getTetrahedronVertices(i);

            newIndices[cnt][0] = oldToNew[oldIndices[0]];
            newIndices[cnt][1] = oldToNew[oldIndices[1]];
            newIndices[cnt][2] = oldToNew[oldIndices[2]];
            newIndices[cnt][3] = oldToNew[oldIndices[3]];

            ++cnt;
        }
    }

    // ready to create the final mesh
    auto mesh = std::make_shared<TetrahedralMesh>();
    mesh->initialize(newCoords, newIndices);

    return mesh;
}

template<typename NeighborContainer>
std::vector<size_t>
GeometryUtils::reorderConnectivity(const std::vector<NeighborContainer>& neighbors, const MeshNodeRenumberingStrategy& method)
{
    switch (method)
    {
    case (MeshNodeRenumberingStrategy::ReverseCuthillMckee):
        return RCM(neighbors);
    default:
        LOG(WARNING) << "Unrecognized reorder method; using RCM instead";
        return RCM(neighbors);
    }
}

template<typename ElemConn>
std::vector<size_t>
GeometryUtils::reorderConnectivity(const std::vector<ElemConn>& conn, const size_t numVerts, const MeshNodeRenumberingStrategy& method)
{
    switch (method)
    {
    case (MeshNodeRenumberingStrategy::ReverseCuthillMckee):
        return RCM(conn, numVerts);
    default:
        LOG(WARNING) << "Unrecognized reorder method; using Reverse Cuthill-Mckee strategy instead";
        return RCM(conn, numVerts);
    }
}
} // namespace imstk

template std::vector<size_t> imstk::GeometryUtils::reorderConnectivity<std::set<size_t>>(const std::vector<std::set<size_t>>&, const GeometryUtils::MeshNodeRenumberingStrategy&);

template std::vector<size_t> imstk::GeometryUtils::reorderConnectivity<std::array<size_t, 4>>(const std::vector<std::array<size_t, 4>>&, const size_t, const MeshNodeRenumberingStrategy&);
