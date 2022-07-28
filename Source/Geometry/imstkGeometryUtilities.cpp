/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometryUtilities.h"
#include "imstkCapsule.h"
#include "imstkCylinder.h"
#include "imstkHexahedralMesh.h"
#include "imstkImageData.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkOrientedBox.h"
#include "imstkParallelUtils.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

#include <vtkCapsuleSource.h>
#include <vtkCellData.h>
#include <vtkCharArray.h>
#include <vtkCleanPolyData.h>
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkDoubleArray.h>
#include <vtkFeatureEdges.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkMassProperties.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkShortArray.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkUnsignedLongLongArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkUnstructuredGrid.h>

static vtkSmartPointer<vtkDataArray>
makeVtkDataArray(unsigned char type)
{
    vtkSmartPointer<vtkDataArray> arr = nullptr;
    switch (type)
    {
    case VTK_CHAR:
        arr = vtkSmartPointer<vtkCharArray>::New();
        break;
    case VTK_UNSIGNED_CHAR:
        arr = vtkSmartPointer<vtkUnsignedCharArray>::New();
        break;
    case VTK_SHORT:
        arr = vtkSmartPointer<vtkShortArray>::New();
        break;
    case VTK_UNSIGNED_SHORT:
        arr = vtkSmartPointer<vtkUnsignedShortArray>::New();
        break;
    case VTK_INT:
        arr = vtkSmartPointer<vtkIntArray>::New();
        break;
    case VTK_UNSIGNED_INT:
        arr = vtkSmartPointer<vtkUnsignedIntArray>::New();
        break;
    case VTK_FLOAT:
        arr = vtkSmartPointer<vtkFloatArray>::New();
        break;
    case VTK_DOUBLE:
        arr = vtkSmartPointer<vtkDoubleArray>::New();
        break;
    case VTK_LONG_LONG:
        arr = vtkSmartPointer<vtkLongLongArray>::New();
        break;
    case VTK_UNSIGNED_LONG:
        arr = vtkSmartPointer<vtkUnsignedLongArray>::New();
        break;
    case VTK_UNSIGNED_LONG_LONG:
        arr = vtkSmartPointer<vtkUnsignedLongLongArray>::New();
    default:
        LOG(WARNING) << "Unknown scalar type";
        break;
    }
    ;
    return arr;
}

namespace imstk
{
vtkSmartPointer<vtkDataArray>
GeometryUtils::coupleVtkDataArray(std::shared_ptr<AbstractDataArray> imstkArray)
{
    CHECK(imstkArray != nullptr) << "AbstractDataArray provided is not valid!";
    CHECK(imstkArray->getVoidPointer() != nullptr) << "AbstractDataArray data provided is not valid!";

    // Create corresponding data array
    vtkSmartPointer<vtkDataArray> arr = makeVtkDataArray(imstkToVtkScalarType[imstkArray->getScalarType()]);
    arr->SetNumberOfComponents(imstkArray->getNumberOfComponents());
    arr->SetVoidArray(imstkArray->getVoidPointer(), imstkArray->size(), 1);

    return arr;
}

vtkSmartPointer<vtkImageData>
GeometryUtils::coupleVtkImageData(std::shared_ptr<ImageData> imageData)
{
    CHECK(imageData != nullptr) << "ImageData provided is not valid!";

    // VTK puts center of min voxel at origin of world space, we put min of bot voxel at origin
    std::shared_ptr<AbstractDataArray> arr    = imageData->getScalars();
    vtkSmartPointer<vtkDataArray>      vtkArr = coupleVtkDataArray(arr);

    vtkSmartPointer<vtkImageData> imageDataVtk = vtkSmartPointer<vtkImageData>::New();
    imageDataVtk->SetDimensions(imageData->getDimensions().data());
    imageDataVtk->SetSpacing(imageData->getSpacing().data());
    const Vec3d vtkOrigin = imageData->getOrigin() + imageData->getSpacing() * 0.5;
    imageDataVtk->SetOrigin(vtkOrigin.data());
    imageDataVtk->SetNumberOfScalarComponents(imageData->getNumComponents(), imageDataVtk->GetInformation());
    imageDataVtk->SetScalarType(imstkToVtkScalarType[arr->getScalarType()], imageDataVtk->GetInformation());
    imageDataVtk->GetPointData()->SetScalars(vtkArr);
    return imageDataVtk;
}

vtkSmartPointer<vtkDataArray>
GeometryUtils::copyToVtkDataArray(std::shared_ptr<AbstractDataArray> imstkArray)
{
    CHECK(imstkArray != nullptr) << "AbstractDataArray provided is not valid!";

    // Create resulting data array
    vtkSmartPointer<vtkDataArray> arr = makeVtkDataArray(imstkToVtkScalarType[imstkArray->getScalarType()]);
    arr->SetNumberOfComponents(imstkArray->getNumberOfComponents());
    arr->SetNumberOfTuples(imstkArray->size() / imstkArray->getNumberOfComponents());
    switch (imstkArray->getScalarType())
    {
        TemplateMacro(std::copy_n(static_cast<IMSTK_TT*>(imstkArray->getVoidPointer()), imstkArray->size(), static_cast<IMSTK_TT*>(arr->GetVoidPointer(0))); );
    default:
        LOG(WARNING) << "Unknown scalar type";
        break;
    }

    return arr;
}

std::shared_ptr<AbstractDataArray>
GeometryUtils::copyToDataArray(vtkSmartPointer<vtkDataArray> vtkArray)
{
    CHECK(vtkArray != nullptr) << "vtkDataArray provided is not valid!";

    std::shared_ptr<AbstractDataArray> arr = nullptr;

    const int numComps = vtkArray->GetNumberOfComponents();

    // Create and copy the array
    switch (vtkToImstkScalarType[vtkArray->GetDataType()])
    {
        TemplateMacro(
            // We enumerate a number of different common # of components
            // There's long reasoning but ultimately this is because we use eigen for DataArrays. limiting
            if (numComps == 1)
            {
                arr = std::make_shared<DataArray<IMSTK_TT>>(vtkArray->GetNumberOfTuples() * vtkArray->GetNumberOfComponents());
                std::copy_n(static_cast<IMSTK_TT*>(vtkArray->GetVoidPointer(0)), vtkArray->GetNumberOfValues(), static_cast<IMSTK_TT*>(arr->getVoidPointer()));
            }
            else if (numComps == 2)
            {
                arr = (std::make_shared<VecDataArray<IMSTK_TT, 2>>(vtkArray->GetNumberOfTuples()));
                std::copy_n(static_cast<IMSTK_TT*>(vtkArray->GetVoidPointer(0)), vtkArray->GetNumberOfValues(), static_cast<IMSTK_TT*>(arr->getVoidPointer()));
            }
            else if (numComps == 3)
            {
                arr = (std::make_shared<VecDataArray<IMSTK_TT, 3>>(vtkArray->GetNumberOfTuples()));
                std::copy_n(static_cast<IMSTK_TT*>(vtkArray->GetVoidPointer(0)), vtkArray->GetNumberOfValues(), static_cast<IMSTK_TT*>(arr->getVoidPointer()));
            }
            else if (numComps == 4)
            {
                arr = (std::make_shared<VecDataArray<IMSTK_TT, 4>>(vtkArray->GetNumberOfTuples()));
                std::copy_n(static_cast<IMSTK_TT*>(vtkArray->GetVoidPointer(0)), vtkArray->GetNumberOfValues(), static_cast<IMSTK_TT*>(arr->getVoidPointer()));
            }
            );
    default:
        LOG(WARNING) << "Unknown scalar type";
        break;
    }
    return arr;
}

std::shared_ptr<ImageData>
GeometryUtils::copyToImageData(vtkSmartPointer<vtkImageData> vtkImage)
{
    CHECK(vtkImage != nullptr) << "vtkImageData provided is not valid!";

    double*     spacingPtr = vtkImage->GetSpacing();
    const Vec3d spacing    = Vec3d(spacingPtr[0], spacingPtr[1], spacingPtr[2]);
    // vtk origin is not bounds and vtk origin is not actual origin
    double* bounds = vtkImage->GetBounds();
    // image data origin starts at center of first voxel
    const Vec3d origin = Vec3d(bounds[0], bounds[2], bounds[4]) - spacing * 0.5;

    std::unique_ptr<ImageData> imageData = std::make_unique<ImageData>();
    imageData->setScalars(copyToDataArray(vtkImage->GetPointData()->GetScalars()),
        vtkImage->GetNumberOfScalarComponents(), vtkImage->GetDimensions());
    imageData->setSpacing(spacing);
    imageData->setOrigin(origin);
    return imageData;
}

vtkSmartPointer<vtkImageData>
GeometryUtils::copyToVtkImageData(std::shared_ptr<ImageData> imageData)
{
    CHECK(imageData != nullptr) << "ImageData provided is not valid!";

    // Our image data does not use vtk extents
    const Vec3i& dim = imageData->getDimensions();

    vtkSmartPointer<vtkImageData> imageDataVtk = vtkSmartPointer<vtkImageData>::New();
    imageDataVtk->SetSpacing(imageData->getSpacing().data());
    const Vec3d vtkOrigin = imageData->getOrigin() + imageData->getSpacing() * 0.5;
    imageDataVtk->SetOrigin(vtkOrigin.data());
    imageDataVtk->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
    imageDataVtk->AllocateScalars(imstkToVtkScalarType[imageData->getScalarType()], imageData->getNumComponents());
    imageDataVtk->GetPointData()->SetScalars(copyToVtkDataArray(imageData->getScalars()));
    return imageDataVtk;
}

std::shared_ptr<PointSet>
GeometryUtils::copyToPointSet(vtkSmartPointer<vtkPointSet> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkPolyData provided is not valid!";

    std::shared_ptr<VecDataArray<double, 3>> vertices = copyToVecDataArray(vtkMesh->GetPoints());

    auto mesh = std::make_unique<PointSet>();
    mesh->initialize(vertices);

    // Point Data
    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> dataMap;
    copyToDataMap(vtkMesh->GetPointData(), dataMap);
    if (!dataMap.empty())
    {
        mesh->setVertexAttributes(dataMap);
        vtkPointData* pointData = vtkMesh->GetPointData();
        vtkDataArray* normals   = pointData->GetNormals();
        vtkDataArray* tCoords   = pointData->GetTCoords();
        vtkDataArray* scalars   = pointData->GetScalars();
        vtkDataArray* tangents  = pointData->GetTangents();
        if (normals != nullptr)
        {
            mesh->setVertexNormals(std::string(normals->GetName()));
        }
        if (tCoords != nullptr)
        {
            mesh->setVertexTCoords(std::string(tCoords->GetName()));
        }
        if (scalars != nullptr)
        {
            mesh->setVertexScalars(std::string(scalars->GetName()));
        }
        if (tangents != nullptr)
        {
            mesh->setVertexTangents(std::string(tangents->GetName()));
        }
    }

    return mesh;
}

std::shared_ptr<SurfaceMesh>
GeometryUtils::copyToSurfaceMesh(vtkSmartPointer<vtkPolyData> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkPolyData provided is not valid!";

    std::shared_ptr<VecDataArray<double, 3>> vertices = copyToVecDataArray(vtkMesh->GetPoints());
    std::shared_ptr<VecDataArray<int, 3>>    cells    = copyToVecDataArray<3>(vtkMesh->GetPolys());

    auto mesh = std::make_unique<SurfaceMesh>();
    mesh->initialize(vertices, cells);

    // Point Data
    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> vertexDataMap;
    copyToDataMap(vtkMesh->GetPointData(), vertexDataMap);
    if (!vertexDataMap.empty())
    {
        mesh->setVertexAttributes(vertexDataMap);
        vtkPointData* pointData = vtkMesh->GetPointData();
        vtkDataArray* normals   = pointData->GetNormals();
        vtkDataArray* tCoords   = pointData->GetTCoords();
        vtkDataArray* scalars   = pointData->GetScalars();
        vtkDataArray* tangents  = pointData->GetTangents();
        if (normals != nullptr)
        {
            mesh->setVertexNormals(std::string(normals->GetName()));
        }
        if (tCoords != nullptr)
        {
            mesh->setVertexTCoords(std::string(tCoords->GetName()));
        }
        if (scalars != nullptr)
        {
            mesh->setVertexScalars(std::string(scalars->GetName()));
        }
        if (tangents != nullptr)
        {
            mesh->setVertexTangents(std::string(tangents->GetName()));
        }
    }

    // Cell Data
    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> cellDataMap;
    copyToDataMap(vtkMesh->GetCellData(), cellDataMap);
    if (!cellDataMap.empty())
    {
        mesh->setCellAttributes(cellDataMap);
        vtkCellData*  cellData = vtkMesh->GetCellData();
        vtkDataArray* normals  = cellData->GetNormals();
        vtkDataArray* scalars  = cellData->GetScalars();
        vtkDataArray* tangents = cellData->GetTangents();
        if (normals != nullptr)
        {
            mesh->setCellNormals(std::string(normals->GetName()));
        }
        if (scalars != nullptr)
        {
            mesh->setCellScalars(std::string(scalars->GetName()));
        }
        if (tangents != nullptr)
        {
            mesh->setCellTangents(std::string(tangents->GetName()));
        }
    }

    // Active Texture
    if (auto pointData = vtkMesh->GetPointData())
    {
        if (auto tcoords = pointData->GetTCoords())
        {
            mesh->setVertexTCoords(tcoords->GetName());
        }
    }

    return mesh;
}

std::shared_ptr<TetrahedralMesh>
GeometryUtils::toTetGrid(
    const Vec3d& center, const Vec3d& size, const Vec3i& dim,
    const Quatd orientation)
{
    auto                     verticesPtr = std::make_shared<VecDataArray<double, 3>>(dim[0] * dim[1] * dim[2]);
    VecDataArray<double, 3>& vertices    = *verticesPtr;
    const Vec3d              dx   = size.cwiseQuotient((dim - Vec3i(1, 1, 1)).cast<double>());
    int                      iter = 0;
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++, iter++)
            {
                vertices[iter] = Vec3i(x, y, z).cast<double>().cwiseProduct(dx) - size * 0.5 + center;
            }
        }
    }

    // Add connectivity data
    auto                  indicesPtr = std::make_shared<VecDataArray<int, 4>>();
    VecDataArray<int, 4>& indices    = *indicesPtr;
    for (int z = 0; z < dim[2] - 1; z++)
    {
        for (int y = 0; y < dim[1] - 1; y++)
        {
            for (int x = 0; x < dim[0] - 1; x++)
            {
                int cubeIndices[8] =
                {
                    x + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * (z + 1))
                };

                // Alternate the pattern so the edges line up on the sides of each voxel
                if ((z % 2 ^ x % 2) ^ y % 2)
                {
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[7], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[2], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[7], cubeIndices[5], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[2], cubeIndices[0], cubeIndices[5]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[6], cubeIndices[7], cubeIndices[5]));
                }
                else
                {
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[3], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[6], cubeIndices[2], cubeIndices[1]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[6], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[3], cubeIndices[1], cubeIndices[4]));
                }
            }
        }
    }

    // Ensure correct windings
    for (int i = 0; i < indices.size(); i++)
    {
        if (tetVolume(vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]]) < 0.0)
        {
            std::swap(indices[i][0], indices[i][2]);
        }
    }

    auto tetMesh = std::make_shared<TetrahedralMesh>();
    tetMesh->initialize(verticesPtr, indicesPtr);
    tetMesh->rotate(orientation, Geometry::TransformType::ApplyToData);
    return tetMesh;
}

std::shared_ptr<SurfaceMesh>
GeometryUtils::toTriangleGrid(
    const Vec3d& center, const Vec2d& size, const Vec2i& dim,
    const Quatd orientation,
    const double uvScale)
{
    auto verticesPtr =
        std::make_shared<VecDataArray<double, 3>>(dim[0] * dim[1]);
    VecDataArray<double, 3>& vertices = *verticesPtr;
    const Vec3d              size3    = Vec3d(size[0], 0.0, size[1]);
    const Vec3i              dim3     = Vec3i(dim[0], 0, dim[1]);
    Vec3d                    dx       = size3.cwiseQuotient((dim3 - Vec3i(1, 0, 1)).cast<double>());
    dx[1] = 0.0;
    int iter = 0;
    for (int y = 0; y < dim[1]; y++)
    {
        for (int x = 0; x < dim[0]; x++, iter++)
        {
            vertices[iter] = Vec3i(x, 0, y).cast<double>().cwiseProduct(dx) + center - size3 * 0.5;
        }
    }

    // Add connectivity data
    auto                  indicesPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>& indices    = *indicesPtr;
    for (int y = 0; y < dim[1] - 1; y++)
    {
        for (int x = 0; x < dim[0] - 1; x++)
        {
            const int index1 = y * dim[0] + x;
            const int index2 = index1 + dim[0];
            const int index3 = index1 + 1;
            const int index4 = index2 + 1;

            // Interleave [/][\]
            if (x % 2 ^ y % 2)
            {
                indices.push_back(Vec3i(index1, index2, index3));
                indices.push_back(Vec3i(index4, index3, index2));
            }
            else
            {
                indices.push_back(Vec3i(index2, index4, index1));
                indices.push_back(Vec3i(index4, index3, index1));
            }
        }
    }

    auto                    uvCoordsPtr = std::make_shared<VecDataArray<float, 2>>(dim[0] * dim[1]);
    VecDataArray<float, 2>& uvCoords    = *uvCoordsPtr.get();
    iter = 0;
    for (int i = 0; i < dim[1]; i++)
    {
        for (int j = 0; j < dim[0]; j++, iter++)
        {
            uvCoords[iter] = Vec2f(
                static_cast<float>(i) / dim[1],
                static_cast<float>(j) / dim[0]) * uvScale;
        }
    }

    auto triMesh = std::make_shared<SurfaceMesh>();
    triMesh->initialize(verticesPtr, indicesPtr);
    triMesh->setVertexTCoords("uvs", uvCoordsPtr);
    triMesh->rotate(orientation, Geometry::TransformType::ApplyToData);
    return triMesh;
}

std::shared_ptr<LineMesh>
GeometryUtils::toLineGrid(
    const Vec3d& start, const Vec3d& dir,
    const double length, const int dim)
{
    auto                     verticesPtr = std::make_shared<VecDataArray<double, 3>>(dim);
    const Vec3d              dirN     = dir.normalized();
    VecDataArray<double, 3>& vertices = *verticesPtr;
    for (int i = 0; i < dim; i++)
    {
        double t = static_cast<double>(i) / (dim - 1);
        vertices[i] = start + dirN * t * length;
    }

    // Add connectivity data
    auto                  indicesPtr = std::make_shared<VecDataArray<int, 2>>();
    VecDataArray<int, 2>& indices    = *indicesPtr;
    for (int i = 0; i < dim - 1; i++)
    {
        indices.push_back(Vec2i(i, i + 1));
    }

    // Create the geometry
    auto lineMesh = std::make_shared<LineMesh>();
    lineMesh->initialize(verticesPtr, indicesPtr);
    return lineMesh;
}

std::shared_ptr<LineMesh>
GeometryUtils::copyToLineMesh(vtkSmartPointer<vtkPolyData> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkPolyData provided is not valid!";

    std::shared_ptr<VecDataArray<double, 3>> vertices = copyToVecDataArray(vtkMesh->GetPoints());
    std::shared_ptr<VecDataArray<int, 2>>    cells    = copyToVecDataArray<2>(vtkMesh->GetPolys());

    // If polys is empty use lines instead
    if (cells->size() == 0)
    {
        cells = copyToVecDataArray<2>(vtkMesh->GetLines());
    }

    auto mesh = std::make_unique<LineMesh>();
    mesh->initialize(vertices, cells);

    // Point Data
    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> dataMap;
    copyToDataMap(vtkMesh->GetPointData(), dataMap);
    if (!dataMap.empty())
    {
        mesh->setVertexAttributes(dataMap);
        vtkPointData* pointData = vtkMesh->GetPointData();
        vtkDataArray* normals   = pointData->GetNormals();
        vtkDataArray* tCoords   = pointData->GetTCoords();
        vtkDataArray* scalars   = pointData->GetScalars();
        vtkDataArray* tangents  = pointData->GetTangents();
        if (normals != nullptr)
        {
            mesh->setVertexNormals(std::string(normals->GetName()));
        }
        if (tCoords != nullptr)
        {
            mesh->setVertexTCoords(std::string(tCoords->GetName()));
        }
        if (scalars != nullptr)
        {
            mesh->setVertexScalars(std::string(scalars->GetName()));
        }
        if (tangents != nullptr)
        {
            mesh->setVertexTangents(std::string(tangents->GetName()));
        }
    }

    // Cell Data
    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> cellDataMap;
    copyToDataMap(vtkMesh->GetCellData(), cellDataMap);
    if (!cellDataMap.empty())
    {
        mesh->setCellAttributes(cellDataMap);
        vtkCellData*  cellData = vtkMesh->GetCellData();
        vtkDataArray* scalars  = cellData->GetScalars();
        if (scalars != nullptr)
        {
            mesh->setCellScalars(std::string(scalars->GetName()));
        }
    }

    return mesh;
}

std::shared_ptr<AbstractCellMesh>
GeometryUtils::copyToCellMesh(vtkSmartPointer<vtkUnstructuredGrid> vtkMesh)
{
    CHECK(vtkMesh != nullptr) << "vtkUnstructuredGrid provided is not valid!";

    std::shared_ptr<VecDataArray<double, 3>> vertices = copyToVecDataArray(vtkMesh->GetPoints());

    const int                         cellType = vtkMesh->GetCellType(vtkMesh->GetNumberOfCells() - 1);
    std::shared_ptr<AbstractCellMesh> vMesh    = nullptr;
    if (cellType == VTK_TETRA)
    {
        std::shared_ptr<VecDataArray<int, 4>> cells = copyToVecDataArray<4>(vtkMesh->GetCells());

        std::shared_ptr<TetrahedralMesh> mesh = std::make_unique<TetrahedralMesh>();
        vMesh = mesh;
        mesh->initialize(vertices, cells);
    }
    else if (cellType == VTK_HEXAHEDRON)
    {
        std::shared_ptr<VecDataArray<int, 8>> cells = copyToVecDataArray<8>(vtkMesh->GetCells());

        std::shared_ptr<HexahedralMesh> mesh = std::make_unique<HexahedralMesh>();
        vMesh = mesh;
        mesh->initialize(vertices, cells);
    }
    else
    {
        LOG(FATAL) << "No support for vtkCellType = "
                   << cellType << ".";
    }

    // Point Data
    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> vertexDataMap;
    copyToDataMap(vtkMesh->GetPointData(), vertexDataMap);
    if (!vertexDataMap.empty())
    {
        vMesh->setVertexAttributes(vertexDataMap);
        vtkPointData* pointData = vtkMesh->GetPointData();
        vtkDataArray* normals   = pointData->GetNormals();
        vtkDataArray* tCoords   = pointData->GetTCoords();
        vtkDataArray* scalars   = pointData->GetScalars();
        vtkDataArray* tangents  = pointData->GetTangents();
        if (normals != nullptr)
        {
            vMesh->setVertexNormals(std::string(normals->GetName()));
        }
        if (tCoords != nullptr)
        {
            vMesh->setVertexTCoords(std::string(tCoords->GetName()));
        }
        if (scalars != nullptr)
        {
            vMesh->setVertexScalars(std::string(scalars->GetName()));
        }
        if (tangents != nullptr)
        {
            vMesh->setVertexTangents(std::string(tangents->GetName()));
        }
    }

    // Cell Data
    /*std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> cellDataMap;
    copyToDataMap(vtkMesh->GetCellData(), cellDataMap);
    if (!cellDataMap.empty())
    {
        vMesh->setCellAttributes(cellDataMap);
        vMesh->setCellNormals(vtkMesh->GetCellData()->GetNormals() == nullptr ? "" : std::string(vtkMesh->GetCellData()->GetNormals()->GetName()));
        vMesh->setCellScalars(vtkMesh->GetCellData()->GetScalars() == nullptr ? "" : std::string(vtkMesh->GetCellData()->GetScalars()->GetName()));
        vMesh->setCellTangents(vtkMesh->GetCellData()->GetTangents() == nullptr ? "" : std::string(vtkMesh->GetCellData()->GetTangents()->GetName()));
    }*/

    return vMesh;
}

vtkSmartPointer<vtkPointSet>
GeometryUtils::copyToVtkPointSet(std::shared_ptr<PointSet> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = copyToVtkPoints(imstkMesh->getVertexPositions());

    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);

    copyToVtkDataAttributes(polydata->GetPointData(), imstkMesh->getVertexAttributes());
    if (imstkMesh->getActiveVertexNormals() != "")
    {
        polydata->GetPointData()->SetActiveNormals(imstkMesh->getActiveVertexNormals().c_str());
    }
    if (imstkMesh->getActiveVertexScalars() != "")
    {
        polydata->GetPointData()->SetActiveScalars(imstkMesh->getActiveVertexScalars().c_str());
    }
    if (imstkMesh->getActiveVertexTangents() != "")
    {
        polydata->GetPointData()->SetActiveTangents(imstkMesh->getActiveVertexTangents().c_str());
    }
    if (imstkMesh->getActiveVertexTCoords() != "")
    {
        polydata->GetPointData()->SetActiveTCoords(imstkMesh->getActiveVertexTCoords().c_str());
    }

    return polydata;
}

vtkSmartPointer<vtkPolyData>
GeometryUtils::copyToVtkPolyData(std::shared_ptr<LineMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints> points = copyToVtkPoints(imstkMesh->getVertexPositions());

    vtkSmartPointer<vtkCellArray> polys = copyToVtkCellArray<2>(imstkMesh->getCells());

    auto polydata = vtkSmartPointer<vtkPolyData>::New();
    polydata->SetPoints(points);
    polydata->SetPolys(polys);

    // Copy vertex attributes
    copyToVtkDataAttributes(polydata->GetPointData(), imstkMesh->getVertexAttributes());
    if (imstkMesh->getActiveVertexNormals() != "")
    {
        polydata->GetPointData()->SetActiveNormals(imstkMesh->getActiveVertexNormals().c_str());
    }
    if (imstkMesh->getActiveVertexScalars() != "")
    {
        polydata->GetPointData()->SetActiveScalars(imstkMesh->getActiveVertexScalars().c_str());
    }
    if (imstkMesh->getActiveVertexTangents() != "")
    {
        polydata->GetPointData()->SetActiveTangents(imstkMesh->getActiveVertexTangents().c_str());
    }
    if (imstkMesh->getActiveVertexTCoords() != "")
    {
        polydata->GetPointData()->SetActiveTCoords(imstkMesh->getActiveVertexTCoords().c_str());
    }

    // Copy cell attributes
    copyToVtkDataAttributes(polydata->GetCellData(), imstkMesh->getCellAttributes());
    if (imstkMesh->getActiveCellScalars() != "")
    {
        polydata->GetCellData()->SetActiveScalars(imstkMesh->getActiveCellScalars().c_str());
    }

    return polydata;
}

vtkSmartPointer<vtkPolyData>
GeometryUtils::copyToVtkPolyData(std::shared_ptr<SurfaceMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints>    points = copyToVtkPoints(imstkMesh->getVertexPositions());
    vtkSmartPointer<vtkCellArray> polys  = copyToVtkCellArray<3>(imstkMesh->getCells());

    auto polydata = vtkSmartPointer<vtkPolyData>::New();;
    polydata->SetPoints(points);
    polydata->SetPolys(polys);

    // Copy vertex attributes
    copyToVtkDataAttributes(polydata->GetPointData(), imstkMesh->getVertexAttributes());
    if (imstkMesh->getActiveVertexNormals() != "")
    {
        polydata->GetPointData()->SetActiveNormals(imstkMesh->getActiveVertexNormals().c_str());
    }
    if (imstkMesh->getActiveVertexScalars() != "")
    {
        polydata->GetPointData()->SetActiveScalars(imstkMesh->getActiveVertexScalars().c_str());
    }
    if (imstkMesh->getActiveVertexTangents() != "")
    {
        polydata->GetPointData()->SetActiveTangents(imstkMesh->getActiveVertexTangents().c_str());
    }
    if (imstkMesh->getActiveVertexTCoords() != "")
    {
        polydata->GetPointData()->SetActiveTCoords(imstkMesh->getActiveVertexTCoords().c_str());
    }

    // Copy cell attributes
    copyToVtkDataAttributes(polydata->GetCellData(), imstkMesh->getCellAttributes());
    if (imstkMesh->getActiveCellNormals() != "")
    {
        polydata->GetCellData()->SetActiveNormals(imstkMesh->getActiveCellNormals().c_str());
    }
    if (imstkMesh->getActiveCellScalars() != "")
    {
        polydata->GetCellData()->SetActiveScalars(imstkMesh->getActiveCellScalars().c_str());
    }
    if (imstkMesh->getActiveCellTangents() != "")
    {
        polydata->GetCellData()->SetActiveTangents(imstkMesh->getActiveCellTangents().c_str());
    }

    return polydata;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::copyToVtkUnstructuredGrid(std::shared_ptr<TetrahedralMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints>    points = copyToVtkPoints(imstkMesh->getVertexPositions());
    vtkSmartPointer<vtkCellArray> tetras = copyToVtkCellArray<4>(imstkMesh->getCells());

    auto ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_TETRA, tetras);

    copyToVtkDataAttributes(ug->GetPointData(), imstkMesh->getVertexAttributes());
    if (imstkMesh->getActiveVertexNormals() != "")
    {
        ug->GetPointData()->SetActiveNormals(imstkMesh->getActiveVertexNormals().c_str());
    }
    if (imstkMesh->getActiveVertexScalars() != "")
    {
        ug->GetPointData()->SetActiveScalars(imstkMesh->getActiveVertexScalars().c_str());
    }
    if (imstkMesh->getActiveVertexTangents() != "")
    {
        ug->GetPointData()->SetActiveTangents(imstkMesh->getActiveVertexTangents().c_str());
    }
    if (imstkMesh->getActiveVertexTCoords() != "")
    {
        ug->GetPointData()->SetActiveTCoords(imstkMesh->getActiveVertexTCoords().c_str());
    }

    // \todo: TetrahedralMeshes don't have cell attributes yet

    return ug;
}

vtkSmartPointer<vtkUnstructuredGrid>
GeometryUtils::copyToVtkUnstructuredGrid(std::shared_ptr<HexahedralMesh> imstkMesh)
{
    vtkSmartPointer<vtkPoints>    points = copyToVtkPoints(imstkMesh->getVertexPositions());
    vtkSmartPointer<vtkCellArray> bricks = copyToVtkCellArray<8>(imstkMesh->getCells());

    auto ug = vtkSmartPointer<vtkUnstructuredGrid>::New();
    ug->SetPoints(points);
    ug->SetCells(VTK_HEXAHEDRON, bricks);

    copyToVtkDataAttributes(ug->GetPointData(), imstkMesh->getVertexAttributes());
    if (imstkMesh->getActiveVertexNormals() != "")
    {
        ug->GetPointData()->SetActiveNormals(imstkMesh->getActiveVertexNormals().c_str());
    }
    if (imstkMesh->getActiveVertexScalars() != "")
    {
        ug->GetPointData()->SetActiveScalars(imstkMesh->getActiveVertexScalars().c_str());
    }
    if (imstkMesh->getActiveVertexTangents() != "")
    {
        ug->GetPointData()->SetActiveTangents(imstkMesh->getActiveVertexTangents().c_str());
    }
    if (imstkMesh->getActiveVertexTCoords() != "")
    {
        ug->GetPointData()->SetActiveTCoords(imstkMesh->getActiveVertexTCoords().c_str());
    }

    // \todo: HexahedralMesh's don't have cell attributes yet

    return ug;
}

std::shared_ptr<VecDataArray<double, 3>>
GeometryUtils::copyToVecDataArray(vtkSmartPointer<vtkPoints> points)
{
    CHECK(points != nullptr) << "No valid point data provided!";

    std::shared_ptr<VecDataArray<double, 3>> vertices   = std::make_shared<VecDataArray<double, 3>>(points->GetNumberOfPoints());
    VecDataArray<double, 3>&                 vertexData = *vertices;
    for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
    {
        double pos[3];
        points->GetPoint(i, pos);
        vertexData[i] = Vec3d(pos[0], pos[1], pos[2]);
    }
    return vertices;
}

vtkSmartPointer<vtkPoints>
GeometryUtils::copyToVtkPoints(std::shared_ptr<VecDataArray<double, 3>> vertices)
{
    vtkSmartPointer<vtkPoints>     points     = vtkSmartPointer<vtkPoints>::New();
    const VecDataArray<double, 3>& vertexData = *vertices;
    points->SetNumberOfPoints(vertexData.size());
    for (int i = 0; i < vertexData.size(); i++)
    {
        points->SetPoint(i, vertexData[i][0], vertexData[i][1], vertexData[i][2]);
    }
    return points;
}

template<size_t dim>
vtkSmartPointer<vtkCellArray>
GeometryUtils::copyToVtkCellArray(std::shared_ptr<VecDataArray<int, dim>> cellsPtr)
{
    vtkSmartPointer<vtkCellArray> vtkCells = vtkSmartPointer<vtkCellArray>::New();

    VecDataArray<int, dim>& cells = *cellsPtr;
    for (int i = 0; i < cells.size(); i++)
    {
        vtkCells->InsertNextCell(dim);
        for (size_t k = 0; k < dim; k++)
        {
            vtkCells->InsertCellPoint(cells[i][k]);
        }
    }
    return vtkCells;
}

template<size_t dim>
std::shared_ptr<VecDataArray<int, dim>>
GeometryUtils::copyToVecDataArray(vtkCellArray* vtkCells)
{
    using ValueType = typename VecDataArray<int, dim>::ValueType;

    CHECK(vtkCells != nullptr) << "No cells found!";

    std::shared_ptr<VecDataArray<int, dim>> indices = std::make_shared<VecDataArray<int, dim>>();
    indices->reserve(vtkCells->GetNumberOfCells());

    vtkNew<vtkIdList> vtkCell;
    vtkCells->InitTraversal();
    while (vtkCells->GetNextCell(vtkCell))
    {
        if (vtkCell->GetNumberOfIds() != dim)
        {
            continue;
        }
        ValueType cell;
        for (size_t i = 0; i < dim; i++)
        {
            cell[i] = vtkCell->GetId(i);
        }
        indices->push_back(cell);
    }
    indices->squeeze();
    return indices;
}

void
GeometryUtils::copyToDataMap(vtkDataSetAttributes* dataAttributes, std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& dataMap)
{
    CHECK(dataAttributes != nullptr) << "No point data provided!";

    for (int i = 0; i < dataAttributes->GetNumberOfArrays(); ++i)
    {
        vtkDataArray* array = dataAttributes->GetArray(i);
        std::string   name  = "unnamed";
        if (array->GetName() == NULL)
        {
            int iter = 0;
            // If name already exists, iterate key
            while (dataMap.count(name + std::to_string(iter)) != 0)
            {
                iter++;
            }
            name = name + std::to_string(iter);
            array->SetName(name.c_str());
        }
        else
        {
            name = std::string(array->GetName());
        }
        dataMap[name] = copyToDataArray(array);
    }
}

void
GeometryUtils::copyToVtkDataAttributes(vtkDataSetAttributes* pointData, const std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& dataMap)
{
    CHECK(pointData != nullptr) << "No point data provided!";

    for (auto i : dataMap)
    {
        vtkSmartPointer<vtkDataArray> arr = copyToVtkDataArray(i.second);
        if (i.first != "")
        {
            arr->SetName(i.first.c_str());
        }
        pointData->AddArray(arr);
    }
}

std::shared_ptr<SurfaceMesh>
GeometryUtils::toUVSphereSurfaceMesh(std::shared_ptr<Sphere> sphere,
                                     const unsigned int phiDivisions, const unsigned int thetaDivisions)
{
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetCenter(sphere->getPosition(Geometry::DataType::PreTransform).data());
    sphereSource->SetRadius(sphere->getRadius());
    sphereSource->SetPhiResolution(phiDivisions);
    sphereSource->SetThetaResolution(thetaDivisions);
    sphereSource->Update();

    vtkNew<vtkTransform> transform;
    transform->SetMatrix(mat4dTranslate(sphere->getPosition()).data());

    vtkNew<vtkTransformFilter> transformFilter;
    transformFilter->SetInputData(sphereSource->GetOutput());
    transformFilter->SetTransform(transform);
    transformFilter->Update();
    vtkNew<vtkTriangleFilter> triangulate;
    triangulate->SetInputData(transformFilter->GetOutput());
    triangulate->Update();
    vtkNew<vtkCleanPolyData> cleanData;
    cleanData->SetInputData(triangulate->GetOutput());
    cleanData->Update();

    return copyToSurfaceMesh(cleanData->GetOutput());
}

std::shared_ptr<SurfaceMesh>
GeometryUtils::toSurfaceMesh(std::shared_ptr<AnalyticalGeometry> geom)
{
    vtkSmartPointer<vtkPointSet> results = nullptr;
    if (auto plane = std::dynamic_pointer_cast<Plane>(geom))
    {
        const Quatd r = Quatd(plane->getRotation());
        const Vec3d i = r._transformVector(Vec3d(1.0, 0.0, 0.0));
        const Vec3d j = r._transformVector(Vec3d(0.0, 0.0, 1.0));

        //Vec3d p1 = plane->getPosition() + plane->getWidth() * (i + j);
        Vec3d p2 = plane->getPosition() + plane->getWidth() * (i - j);
        Vec3d p3 = plane->getPosition() + plane->getWidth() * (-i + j);
        Vec3d p4 = plane->getPosition() + plane->getWidth() * (-i - j);

        vtkNew<vtkPlaneSource> source;
        source->SetOrigin(p4.data());
        source->SetPoint1(p3.data());
        source->SetPoint2(p2.data());
        source->Update();
        results = source->GetOutput();
    }
    else if (auto orientedBox = std::dynamic_pointer_cast<OrientedBox>(geom))
    {
        vtkNew<vtkCubeSource> source;
        Vec3d                 extents = orientedBox->getExtents(Geometry::DataType::PreTransform);
        source->SetCenter(0.0, 0.0, 0.0);
        source->SetXLength(extents[0] * 2.0);
        source->SetYLength(extents[1] * 2.0);
        source->SetZLength(extents[2] * 2.0);
        source->Update();

        AffineTransform3d T = AffineTransform3d::Identity();
        T.translate(orientedBox->getPosition(Geometry::DataType::PostTransform));
        T.rotate(orientedBox->getOrientation(Geometry::DataType::PostTransform));
        T.scale(orientedBox->getScaling());
        T.matrix().transposeInPlace();

        vtkNew<vtkTransform> transformVtk;
        transformVtk->SetMatrix(T.data());

        vtkNew<vtkTransformFilter> transformFilter;
        transformFilter->SetInputData(source->GetOutput());
        transformFilter->SetTransform(transformVtk);
        transformFilter->Update();
        results = transformFilter->GetOutput();
    }
    else if (auto cylinder = std::dynamic_pointer_cast<Cylinder>(geom))
    {
        vtkNew<vtkCylinderSource> source;
        source->SetCenter(0.0, 0.0, 0.0);
        source->SetRadius(cylinder->getRadius());
        source->SetHeight(cylinder->getLength());
        source->SetResolution(20);
        source->Update();

        AffineTransform3d T = AffineTransform3d::Identity();
        T.translate(cylinder->getPosition(Geometry::DataType::PostTransform));
        T.rotate(cylinder->getOrientation(Geometry::DataType::PostTransform));
        T.scale(1.0);
        T.matrix().transposeInPlace();

        vtkNew<vtkTransform> transformVtk;
        transformVtk->SetMatrix(T.data());

        vtkNew<vtkTransformFilter> transformFilter;
        transformFilter->SetInputData(source->GetOutput());
        transformFilter->SetTransform(transformVtk);
        transformFilter->Update();
        results = transformFilter->GetOutput();
    }
    else if (auto capsule = std::dynamic_pointer_cast<Capsule>(geom))
    {
        vtkNew<vtkCapsuleSource> source;
        source->SetCenter(0.0, 0.0, 0.0);
        source->SetRadius(capsule->getRadius());
        source->SetCylinderLength(capsule->getLength());
        source->SetLatLongTessellation(20);
        source->SetPhiResolution(20);
        source->SetThetaResolution(20);
        source->Update();

        AffineTransform3d T = AffineTransform3d::Identity();
        T.translate(capsule->getPosition(Geometry::DataType::PostTransform));
        T.rotate(capsule->getOrientation(Geometry::DataType::PostTransform));
        T.scale(1.0);
        T.matrix().transposeInPlace();

        vtkNew<vtkTransform> transformVtk;
        transformVtk->SetMatrix(T.data());

        vtkNew<vtkTransformFilter> transformFilter;
        transformFilter->SetInputData(source->GetOutput());
        transformFilter->SetTransform(transformVtk);
        transformFilter->Update();
        results = transformFilter->GetOutput();
    }
    else
    {
        LOG(WARNING) << "Failed to produce SurfaceMesh from provided AnalyticalGeometry";
        return nullptr;
    }

    // Triangulate, mesh could have quads or other primitives
    vtkNew<vtkTriangleFilter> triangulate;
    triangulate->SetInputData(results);
    triangulate->Update();
    vtkNew<vtkCleanPolyData> cleanData;
    cleanData->SetInputConnection(triangulate->GetOutputPort());
    cleanData->Update();
    return copyToSurfaceMesh(cleanData->GetOutput());
}

int
GeometryUtils::getOpenEdgeCount(std::shared_ptr<SurfaceMesh> surfMesh)
{
    vtkNew<vtkFeatureEdges> checkClosed;
    checkClosed->SetInputData(GeometryUtils::copyToVtkPolyData(surfMesh));
    checkClosed->FeatureEdgesOff();
    checkClosed->BoundaryEdgesOn();
    checkClosed->NonManifoldEdgesOn();
    checkClosed->Update();
    return checkClosed->GetOutput()->GetNumberOfCells();
}

double
GeometryUtils::getVolume(std::shared_ptr<SurfaceMesh> surfMesh)
{
    vtkNew<vtkMassProperties> massProps;
    massProps->SetInputData(copyToVtkPolyData(surfMesh));
    massProps->Update();
    return massProps->GetVolume();
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
                       SurfaceMesh&            surfaceMesh,
                       const StdVectorOfVec3d& coords)
{
    isInside.clear();
    isInside.resize(coords.size(), false);

    Vec3d aabbMin, aabbMax;
    surfaceMesh.computeBoundingBox(aabbMin, aabbMax, 1.0);
    /*
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
    */

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

    bBoxMin.resize(surfaceMesh.getNumCells());
    bBoxMax.resize(surfaceMesh.getNumCells());

    const VecDataArray<int, 3>& indices = *surfaceMesh.getCells();
    for (int idx = 0; idx < surfaceMesh.getNumCells(); ++idx)
    {
        const auto& verts = indices[idx];
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

    auto rayTracingFunc = [&](const size_t i)
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
                              const auto& xyz = *surfaceMesh.getVertexPositions();

                              for (int j = 0; j < surfaceMesh.getNumCells(); ++j)
                              {
                                  const Vec3i& verts = indices[j];

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
/// It makes uses of ray-casting but skips points based on the nearest distance between current point and the surface.
///
/// \param surfaceMesh a \ref SurfaceMesh
/// \param coords a set of points to be tested
/// \param nx number of points in x-direction
/// \param ny number of points in y-direction
/// \param nz number of points in z-direction
///
void
markPointsInsideAndOut(std::vector<bool>& isInside,
                       SurfaceMesh& surfaceMesh,
                       const VecDataArray<double, 3>& coords,
                       const size_t nx,
                       const size_t ny,
                       const size_t nz)
{
    isInside.clear();
    isInside.resize(coords.size(), false);

    Vec3d aabbMin, aabbMax;
    surfaceMesh.computeBoundingBox(aabbMin, aabbMax, 1.0);
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

    bBoxMin.resize(surfaceMesh.getNumCells());
    bBoxMax.resize(surfaceMesh.getNumCells());

    /// \brief find the bounding boxes of each surface triangle
    const VecDataArray<int, 3>& indices = *surfaceMesh.getCells();
    auto                        findBoundingBox = [&](const size_t idx)
                                                  {
                                                      const auto& verts   = indices[idx];
                                                      const auto& vertXyz = *surfaceMesh.getVertexPositions();
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

    ParallelUtils::parallelFor(surfaceMesh.getNumCells(), findBoundingBox);

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
                              const auto& xyz       = *surfaceMesh.getVertexPositions();

                              size_t i = 0;
                              while (i < nx)
                              {
                                  size_t idx = idx0 + i;
                                  int    numIntersections = 0;
                                  double dist    = 0.0;
                                  double distMin = h[0] * (nz + 1);

                                  for (int j = 0; j < surfaceMesh.getNumCells(); ++j)
                                  {
                                      const Vec3i& verts = indices[j];

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
}   // anonymous namespace

std::shared_ptr<TetrahedralMesh>
GeometryUtils::createUniformMesh(const Vec3d& aabbMin,
                                 const Vec3d& aabbMax,
                                 const int    nx,
                                 const int    ny,
                                 const int    nz)
{
    const Vec3d h = { (aabbMax[0] - aabbMin[0]) / nx,
                      (aabbMax[1] - aabbMin[1]) / ny,
                      (aabbMax[2] - aabbMin[2]) / nz };
    LOG_IF(FATAL, (h[0] <= 0.0 || h[1] <= 0.0 || h[2] <= 0.0)) << "Invalid bounding box";

    const size_t numVertices = (nx + 1) * (ny + 1) * (nz + 1);

    // std::vector<Vec3d> coords;
    std::shared_ptr<VecDataArray<double, 3>> coords   = std::make_shared<VecDataArray<double, 3>>();
    VecDataArray<double, 3>&                 vertices = *coords;
    vertices.resize(static_cast<int>(numVertices));
    int cnt = 0;

    for (int k = 0; k < nz + 1; ++k)
    {
        double z = aabbMin[2] + k * h[2];
        for (int j = 0; j < ny + 1; ++j)
        {
            double y = aabbMin[1] + j * h[1];
            for (int i = 0; i < nx + 1; ++i)
            {
                double x = aabbMin[0] + i * h[0];
                vertices[cnt] = { x, y, z };
                ++cnt;
            }
        }
    }

    const int numDiv  = 6;
    const int numTets = numDiv * nx * ny * nz;

    std::shared_ptr<VecDataArray<int, 4>> indicesPtr = std::make_shared<VecDataArray<int, 4>>();
    VecDataArray<int, 4>&                 indices    = *indicesPtr;
    indices.resize(static_cast<int>(numTets));
    cnt = 0;
    std::vector<int> indx(8);

    for (int k = 0; k < nz; ++k)
    {
        for (int j = 0; j < ny; ++j)
        {
            for (int i = 0; i < nx; ++i)
            {
                indx[3] = i + j * (nx + 1) + k * (nx + 1) * (ny + 1);
                indx[2] = indx[3] + 1;
                indx[0] = indx[3] + nx + 1;
                indx[1] = indx[0] + 1;
                indx[4] = indx[0] + (nx + 1) * (ny + 1);
                indx[5] = indx[1] + (nx + 1) * (ny + 1);
                indx[6] = indx[2] + (nx + 1) * (ny + 1);
                indx[7] = indx[3] + (nx + 1) * (ny + 1);

                indices[cnt + 0] = Vec4i(indx[0], indx[2], indx[3], indx[6]);
                indices[cnt + 1] = Vec4i(indx[0], indx[3], indx[7], indx[6]);
                indices[cnt + 2] = Vec4i(indx[0], indx[7], indx[4], indx[6]);
                indices[cnt + 3] = Vec4i(indx[0], indx[5], indx[6], indx[4]);
                indices[cnt + 4] = Vec4i(indx[1], indx[5], indx[6], indx[0]);
                indices[cnt + 5] = Vec4i(indx[1], indx[6], indx[2], indx[0]);
                cnt += numDiv;
            }
        }
    }

    auto mesh = std::make_shared<TetrahedralMesh>();
    mesh->initialize(coords, indicesPtr);
    return mesh;
}

std::shared_ptr<TetrahedralMesh>
GeometryUtils::createTetrahedralMeshCover(std::shared_ptr<SurfaceMesh> surfMesh,
                                          const int                    nx,
                                          const int                    ny,
                                          const int                    nz)
{
    Vec3d aabbMin, aabbMax;

    // create a background mesh
    surfMesh->computeBoundingBox(aabbMin, aabbMax, 1. /*percentage padding*/);
    auto uniformMesh = createUniformMesh(aabbMin, aabbMax, nx, ny, nz);

    // ray-tracing
    const VecDataArray<double, 3>& coords = *uniformMesh->getVertexPositions();
    std::vector<bool>              insideSurfMesh;
    markPointsInsideAndOut(insideSurfMesh, *surfMesh.get(), coords, nx + 1, ny + 1, nz + 1);

    // label elements
    std::vector<bool> validTet(uniformMesh->getNumCells(), false);
    std::vector<bool> validVtx(uniformMesh->getNumVertices(), false);

    // TetrahedralMesh::WeightsArray weights = {0.0, 0.0, 0.0, 0.0};
    const Vec3d h = { (aabbMax[0] - aabbMin[0]) / nx,
                      (aabbMax[1] - aabbMin[1]) / ny,
                      (aabbMax[2] - aabbMin[2]) / nz };

    // a customized approach to find the enclosing tet for each surface points
    /// \todo can be parallelized by make NUM_THREADS copies of validTet, or use atomic op on validTet
    auto labelEnclosingTet = [&aabbMin, &h, nx, ny, nz, &uniformMesh, &validTet](const Vec3d& xyz)
                             {
                                 const int idX   = (xyz[0] - aabbMin[0]) / h[0];
                                 const int idY   = (xyz[1] - aabbMin[1]) / h[1];
                                 const int idZ   = (xyz[2] - aabbMin[2]) / h[2];
                                 const int hexId = idX + idY * nx + idZ * nx * ny;

                                 // the index range of tets inside the enclosing hex
                                 const int numDiv = 6;
                                 const int tetId0 = numDiv * hexId;
                                 const int tetId1 = tetId0 + numDiv;

                                 static Vec4d weights = Vec4d::Zero();

                                 // loop over the tets to find the enclosing tets
                                 for (int id = tetId0; id < tetId1; ++id)
                                 {
                                     if (validTet[id])
                                     {
                                         continue;
                                     }
                                     weights = uniformMesh->computeBarycentricWeights(id, xyz);

                                     if ((weights[0] >= 0) && (weights[1] >= 0) && (weights[2] >= 0) && (weights[3] >= 0))
                                     {
                                         validTet[id] = true;
                                         break;
                                     }
                                 }
                             };

    auto labelEnclosingTetOfVertices = [&surfMesh, &uniformMesh, &aabbMin, &h, nx, ny, nz, &labelEnclosingTet, &validTet](const int i)
                                       {
                                           const auto& xyz = surfMesh->getVertexPosition(i);
                                           labelEnclosingTet(xyz);
                                       };

    for (size_t i = 0; i < validTet.size(); ++i)
    {
        const Vec4i& verts = (*uniformMesh->getCells())[i];
        if (insideSurfMesh[verts[0]]
            || insideSurfMesh[verts[1]]
            || insideSurfMesh[verts[2]]
            || insideSurfMesh[verts[3]])
        {
            validTet[i] = true;
        }
    }

    // find the enclosing tets of a group of points on a surface triangle
    const VecDataArray<int, 3>& indices = *surfMesh->getCells();
    auto                        labelEnclosingTetOfInteriorPnt = [&](const int fid)
                                                                 {
                                                                     auto               verts = indices[fid];
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
    for (int i = 0; i < surfMesh->getNumVertices(); ++i)
    {
        labelEnclosingTetOfVertices(i);
    }

    // enclose some interior points on triangles
    for (int i = 0; i < surfMesh->getNumCells(); ++i)
    {
        labelEnclosingTetOfInteriorPnt(i);
    }

    int numElems = 0;
    for (size_t i = 0; i < validTet.size(); ++i)
    {
        const Vec4i& verts = (*uniformMesh->getCells())[i];
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

    std::shared_ptr<VecDataArray<double, 3>> newCoords = std::make_shared<VecDataArray<double, 3>>(static_cast<int>(numVerts));
    VecDataArray<double, 3>&                 vertices = *newCoords;
    std::vector<int>                         oldToNew(coords.size(), std::numeric_limits<int>::max());

    int cnt = 0;
    for (size_t i = 0; i < validVtx.size(); ++i)
    {
        if (validVtx[i])
        {
            vertices[cnt] = coords[i];
            oldToNew[i]   = cnt;
            ++cnt;
        }
    }

    // update tet-to-vert connectivity
    std::shared_ptr<VecDataArray<int, 4>> newIndicesPtr = std::make_shared<VecDataArray<int, 4>>(numElems);
    VecDataArray<int, 4>&                 newIndices = *newIndicesPtr;
    cnt = 0;
    for (int i = 0; i < uniformMesh->getNumCells(); ++i)
    {
        if (validTet[i])
        {
            const Vec4i& oldIndices = (*uniformMesh->getCells())[i];

            newIndices[cnt][0] = oldToNew[oldIndices[0]];
            newIndices[cnt][1] = oldToNew[oldIndices[1]];
            newIndices[cnt][2] = oldToNew[oldIndices[2]];
            newIndices[cnt][3] = oldToNew[oldIndices[3]];

            ++cnt;
        }
    }

    // ready to create the final mesh
    auto mesh = std::make_shared<TetrahedralMesh>();
    mesh->initialize(newCoords, newIndicesPtr);

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
