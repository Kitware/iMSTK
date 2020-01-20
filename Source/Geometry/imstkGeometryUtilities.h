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

#include "imstkLineMesh.h"
#include "imstkSurfaceMesh.h"
#include <memory>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>
#include <vtkCleanPolyData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace imstk
{

namespace geometryutils
{

///
/// \brief Converts vtkPolyData to imstk SurfaceMesh, vtkPolyData must be triangles only
///
static std::shared_ptr<SurfaceMesh>
vtkToImstkSurfaceMesh(vtkSmartPointer<vtkPolyData> polyData)
{
    vtkPoints* pts = polyData->GetPoints();
    StdVectorOfVec3d vertList;
    vertList.resize(pts->GetNumberOfPoints());
    for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        vertList[i] = Vec3d(pt);
    }

    vtkCellArray* cells = polyData->GetPolys();
    cells->InitTraversal();
    vtkNew<vtkIdList> idList;
    std::vector<SurfaceMesh::TriangleArray> triangles(cells->GetNumberOfCells());
    for (vtkIdType i = 0; i < cells->GetNumberOfCells(); i++)
    {
        cells->GetNextCell(idList);
        vtkIdType numIds = idList->GetNumberOfIds();
        if (idList->GetNumberOfIds() != 3)
        {
            LOG(FATAL) << "vtkPolyData contains non-triangle cell";
        }
        else
        {
            triangles[i] = {
                static_cast<unsigned int>(idList->GetId(0)),
                static_cast<unsigned int>(idList->GetId(1)),
                static_cast<unsigned int>(idList->GetId(2)) };
        }
    }

    std::shared_ptr<SurfaceMesh> surfMesh = std::make_shared<SurfaceMesh>();
    surfMesh->setTrianglesVertices(triangles);
    surfMesh->setInitialVertexPositions(vertList);
    surfMesh->setVertexPositions(vertList);

    return surfMesh;
}

///
/// \brief Converts imstk SurfaceMesh to vtkPolyData
///
static vtkSmartPointer<vtkPolyData>
imstkToVtkSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh)
{
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

    vtkPoints* pts = vtkPoints::New();
    pts->SetNumberOfPoints(surfaceMesh->getNumVertices());
    for (size_t i = 0; i < surfaceMesh->getNumVertices(); i++)
    {
        pts->SetPoint(static_cast<vtkIdType>(i), surfaceMesh->getVertexPosition(i).data());
    }

    vtkCellArray* cells = vtkCellArray::New();
    std::vector<SurfaceMesh::TriangleArray> elements = surfaceMesh->getTrianglesVertices();
    vtkIdType tri[3];
    for (size_t i = 0; i < elements.size(); i++)
    {
        tri[0] = elements[i][0];
        tri[1] = elements[i][1];
        tri[2] = elements[i][2];
        cells->InsertNextCell(3, tri);
    }
    polyData->SetPoints(pts);
    polyData->SetPolys(cells);

    pts->Delete();
    cells->Delete();

    return polyData;
}

///
/// \brief Converts vtkPolyData to imstk LineMesh
///
static std::shared_ptr<LineMesh>
vtkToImstkLineMesh(vtkSmartPointer<vtkPolyData> polyData)
{
    vtkPoints* pts = polyData->GetPoints();
    StdVectorOfVec3d vertList;
    vertList.resize(pts->GetNumberOfPoints());
    for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); i++)
    {
        double pt[3];
        pts->GetPoint(i, pt);
        vertList[i] = Vec3d(pt);
    }

    vtkCellArray* cells = polyData->GetPolys();
    cells->InitTraversal();
    vtkNew<vtkIdList> idList;
    std::vector<LineMesh::LineArray> lines(cells->GetNumberOfCells());
    for (vtkIdType i = 0; i < cells->GetNumberOfCells(); i++)
    {
        cells->GetNextCell(idList);
        vtkIdType numIds = idList->GetNumberOfIds();
        if (idList->GetNumberOfIds() != 2)
        {
            LOG(FATAL) << "vtkPolyData contains a non-line cell";
        }
        else
        {
            lines[i] = {
                static_cast<unsigned int>(idList->GetId(0)),
                static_cast<unsigned int>(idList->GetId(1)) };
        }
    }

    std::shared_ptr<LineMesh> lineMesh = std::make_shared<LineMesh>();
    lineMesh->setLinesVertices(lines);
    lineMesh->setInitialVertexPositions(vertList);
    lineMesh->setVertexPositions(vertList);

    return lineMesh;
}

///
/// \brief Converts imstk LineMesh to vtkPolyData 
///
static vtkSmartPointer<vtkPolyData> imstktoVtkLineMesh(std::shared_ptr<LineMesh> lineMesh)
{
    vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

    vtkPoints* pts = vtkPoints::New();
    pts->SetNumberOfPoints(lineMesh->getNumVertices());
    for (size_t i = 0; i < lineMesh->getNumVertices(); i++)
    {
        pts->SetPoint(static_cast<vtkIdType>(i), lineMesh->getVertexPosition(i).data());
    }

    vtkCellArray* cells = vtkCellArray::New();
    std::vector<LineMesh::LineArray> elements = lineMesh->getLinesVertices();
    vtkIdType seg[2];
    for (size_t i = 0; i < elements.size(); i++)
    {
        seg[0] = elements[i][0];
        seg[1] = elements[i][1];
        cells->InsertNextCell(2, seg);
    }
    polyData->SetPoints(pts);
    polyData->SetPolys(cells);

    pts->Delete();
    cells->Delete();

    return polyData;
}

///
/// \brief Appends two surface meshes
///
static std::shared_ptr<SurfaceMesh>
appendSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh1, std::shared_ptr<SurfaceMesh> surfaceMesh2)
{
    vtkSmartPointer<vtkAppendPolyData> filter = vtkSmartPointer<vtkAppendPolyData>::New();
    filter->AddInputData(imstkToVtkSurfaceMesh(surfaceMesh1));
    filter->AddInputData(imstkToVtkSurfaceMesh(surfaceMesh2));
    filter->Update();
    return vtkToImstkSurfaceMesh(filter->GetOutput());
}
}
}
