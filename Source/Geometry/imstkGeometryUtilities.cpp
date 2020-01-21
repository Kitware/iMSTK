#include "imstkGeometryUtilities.h"

namespace imstk
{
std::shared_ptr<SurfaceMesh>
GeometryUtils::convertVtkPolyDataToSurfaceMesh(vtkSmartPointer<vtkPolyData> vtkMesh)
{
    if (!vtkMesh)
    {
        LOG(FATAL) << "VTKMeshIO::convertVtkPolyDataToSurfaceMesh error: could not read with VTK reader.";
        return nullptr;
    }

    StdVectorOfVec3d vertices;
    copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    std::vector<SurfaceMesh::TriangleArray> triangles;
    copyCellsFromVtk<3>(vtkMesh->GetPolys(), triangles);

    auto mesh = std::make_shared<SurfaceMesh>();
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

std::shared_ptr<LineMesh>
GeometryUtils::convertVtkPolyDataToLineMesh(vtkSmartPointer<vtkPolyData> vtkMesh)
{
    if (!vtkMesh)
    {
        LOG(FATAL) << "VTKMeshIO::convertVtkPolyDataToSurfaceMesh error: could not read with VTK reader.";
        return nullptr;
    }

    StdVectorOfVec3d vertices;
    copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    std::vector<LineMesh::LineArray> segments;
    copyCellsFromVtk<2>(vtkMesh->GetPolys(), segments);

    auto mesh = std::make_shared<LineMesh>();
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
GeometryUtils::convertSurfaceMeshToVtkPolyData(std::shared_ptr<SurfaceMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    copyCellsToVtk<3>(imstkMesh->getTrianglesVertices(), polys.Get());

    vtkPolyData* polydata = vtkPolyData::New();
    polydata->SetPoints(points);
    polydata->SetPolys(polys);
    return polydata;
}

vtkSmartPointer<vtkPolyData>
GeometryUtils::convertLineMeshToVtkPolyData(std::shared_ptr<LineMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
    copyCellsToVtk<2>(imstkMesh->getLinesVertices(), polys.Get());

    vtkPolyData* polydata = vtkPolyData::New();
    polydata->SetPoints(points);
    polydata->SetPolys(polys);
    return polydata;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::convertTetrahedralMeshToVtkUnstructuredGrid(std::shared_ptr<TetrahedralMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkSmartPointer<vtkCellArray> tetras = vtkSmartPointer<vtkCellArray>::New();
    copyCellsToVtk<4>(imstkMesh->getTetrahedraVertices(), tetras.Get());

    vtkUnstructuredGrid* ug = vtkUnstructuredGrid::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_TETRA, tetras);
    return ug;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::convertHexahedralMeshToVtkUnstructuredGrid(std::shared_ptr<HexahedralMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    copyVerticesToVtk(imstkMesh->getVertexPositions(), points.Get());

    vtkSmartPointer<vtkCellArray> bricks = vtkSmartPointer<vtkCellArray>::New();
    copyCellsToVtk<8>(imstkMesh->getHexahedraVertices(), bricks.Get());

    vtkUnstructuredGrid* ug = vtkUnstructuredGrid::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_HEXAHEDRON, bricks);
    return ug;
}

std::shared_ptr<VolumetricMesh>
GeometryUtils::convertVtkUnstructuredGridToVolumetricMesh(vtkUnstructuredGrid* vtkMesh)
{
    if (!vtkMesh)
    {
        LOG(FATAL) << "VTKMeshIO::convertVtkUnstructuredGridToVolumetricMesh error: could not read with VTK reader.";
        return nullptr;
    }

    StdVectorOfVec3d vertices;
    copyVerticesFromVtk(vtkMesh->GetPoints(), vertices);

    int cellType = vtkMesh->GetCellType(vtkMesh->GetNumberOfCells() - 1);
    if (cellType == VTK_TETRA)
    {
        std::vector<TetrahedralMesh::TetraArray> cells;
        copyCellsFromVtk<4>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_shared<TetrahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else if (cellType == VTK_HEXAHEDRON)
    {
        std::vector<HexahedralMesh::HexaArray> cells;
        copyCellsFromVtk<8>(vtkMesh->GetCells(), cells);

        auto mesh = std::make_shared<HexahedralMesh>();
        mesh->initialize(vertices, cells, false);
        return mesh;
    }
    else
    {
        LOG(FATAL) << "VTKMeshIO::convertVtkUnstructuredGridToVolumetricMesh error: No support for vtkCellType="
            << cellType << ".";
        return nullptr;
    }
}

void
GeometryUtils::copyVerticesFromVtk(vtkPoints* points, StdVectorOfVec3d& vertices)
{
    if (!points)
    {
        LOG(FATAL) << "VTKMeshIO::copyVerticesFromVtk error: No points found.";
        return;
    }

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
    if (!points)
    {
        LOG(FATAL) << "VTKMeshIO::copyVerticesToVtk error: No points found.";
        return;
    }

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
    if (!vtkCells)
    {
        LOG(FATAL) << "VTKMeshIO::copyCellsToVtk error: No cells found.";
        return;
    }

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
    if (!vtkCells)
    {
        LOG(FATAL) << "VTKMeshIO::copyCellsFromVtk error: No cells found.";
        return;
    }

    cells.reserve(vtkCells->GetNumberOfCells());
    vtkCells->InitTraversal();
    auto                    vtkCell = vtkSmartPointer<vtkIdList>::New();
    std::array<size_t, dim> cell;
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
GeometryUtils::copyPointDataFromVtk(vtkPointData* pointData, std::map<std::string, StdVectorOfVectorf>& dataMap)
{
    if (!pointData)
    {
        return;
    }

    for (int i = 0; i < pointData->GetNumberOfArrays(); ++i)
    {
        vtkDataArray* array = pointData->GetArray(i);
        std::string        name = array->GetName();
        int                nbrOfComp = array->GetNumberOfComponents();
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

std::shared_ptr<SurfaceMesh>
GeometryUtils::appendSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh1, std::shared_ptr<SurfaceMesh> surfaceMesh2)
{
    vtkSmartPointer<vtkAppendPolyData> filter = vtkSmartPointer<vtkAppendPolyData>::New();
    filter->AddInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh1));
    filter->AddInputData(convertSurfaceMeshToVtkPolyData(surfaceMesh2));
    filter->Update();
    return convertVtkPolyDataToSurfaceMesh(filter->GetOutput());
}

std::shared_ptr<LineMesh>
GeometryUtils::SurfaceMeshToLineMesh(std::shared_ptr<SurfaceMesh> surfMesh)
{
    vtkSmartPointer<vtkExtractEdges> filter = vtkSmartPointer<vtkExtractEdges>::New();
    filter->SetInputData(convertSurfaceMeshToVtkPolyData(surfMesh));
    filter->Update();
    return convertVtkPolyDataToLineMesh(filter->GetOutput());
}
}