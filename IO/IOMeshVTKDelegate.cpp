
// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include <array>
#include <vector>

// SimMedTK includes
#include "IO/IOMeshDelegate.h"
#include "Core/Factory.h"

// VTK includes
#include <vtkNew.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedIntArray.h>
#include <vtkCellArray.h>
#include <vtkFieldData.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkSTLReader.h>

class VTKMeshDelegate : public IOMeshDelegate
{
public:
    ///
    /// \brief
    ///
    void read()
    {
        auto name = this->meshIO->getFileName().c_str();
        vtkPoints *points;
        vtkCellArray *cellArray;
        vtkFieldData *fieldData;

        std::vector<core::Vec3d> vertices;
        std::vector<std::array<size_t,3>> triangleArray;
        std::vector<std::array<size_t,4>> tetraArray;
        std::vector<std::array<size_t,8>> hexaArray;
        std::vector<size_t> bdConditions;
        core::Vec3d materials;

        this->meshProps = 0;

        switch(this->meshIO->getFileType())
        {
            case IOMesh::MeshFileType::OBJ:
            {
                vtkNew<vtkOBJReader> reader;
                reader->SetFileName(name);
                reader->Update();
                points = reader->GetOutput()->GetPoints();
                cellArray = reader->GetOutput()->GetPolys();
                fieldData = reader->GetOutput()->GetFieldData();
                this->meshProps |= MeshType::Tri;
                break;
            }
            case IOMesh::MeshFileType::STL:
            {
                vtkNew<vtkSTLReader> reader;
                reader->SetFileName(name);
                reader->Update();
                points = reader->GetOutput()->GetPoints();
                cellArray = reader->GetOutput()->GetPolys();
                fieldData = reader->GetOutput()->GetFieldData();
                this->meshProps |= MeshType::Tri;
                break;
            }
            case IOMesh::MeshFileType::PLY:
            {
                vtkNew<vtkPLYReader> reader;
                reader->SetFileName(name);
                reader->Update();
                points = reader->GetOutput()->GetPoints();
                cellArray = reader->GetOutput()->GetPolys();
                fieldData = reader->GetOutput()->GetFieldData();
                this->meshProps |= MeshType::Tri;
                break;
            }
            default:
            {
                vtkNew<vtkGenericDataObjectReader> reader;
                reader->SetFileName(name);
                reader->Update();

                vtkPointSet *output = vtkPointSet::SafeDownCast(reader->GetOutput());

                if(!output)
                {
                    std::cerr << "VTKMeshReaderDelegate: Empty reader for vtk files." << std::endl;
                    return;
                }

                points = output->GetPoints();
                fieldData = output->GetFieldData();
                if(reader->IsFilePolyData())
                {
                    cellArray = reader->GetPolyDataOutput()->GetPolys();
                }
                else if(reader->IsFileUnstructuredGrid())
                {
                    cellArray = reader->GetUnstructuredGridOutput()->GetCells();
                }
                else
                {
                    std::cerr << "VTKMeshReaderDelegate: Unsupported dataset." << std::endl;
                }
            }
        }
        this->copyPoints(points,vertices);
        this->copyCells(cellArray,triangleArray,tetraArray,hexaArray);
        this->copyData(fieldData,bdConditions,materials);

        //--
        if(this->meshProps & MeshType::Tetra)
        {
            if((this->meshProps & MeshType::hasBDConditions) && (this->meshProps & MeshType::hasMaterials))
            {
                this->setVegaTetraMesh(vertices,tetraArray,bdConditions,materials);
            }
            else
            {
                this->setVegaTetraMesh(vertices,tetraArray);
            }
            if(this->meshProps & MeshType::Tri)
            {
                // If the vtk dataset also contains surface triangles, store them in the mesh
                std::vector<core::Vec3d> surfaceVertices;
                std::unordered_map<size_t,size_t> uniqueVertexArray;
                this->reorderSurfaceTopology(triangleArray,vertices,surfaceVertices,uniqueVertexArray);

                auto meshToAttach = std::make_shared<SurfaceMesh>();
                meshToAttach->setVertices(surfaceVertices);
                meshToAttach->setTriangles(triangleArray);

                auto vegaMesh = std::static_pointer_cast<VegaVolumetricMesh>(
                    this->meshIO->getMesh());
                vegaMesh->setVertexMap(uniqueVertexArray);
                vegaMesh->attachSurfaceMesh(meshToAttach);
            }
            else
            {
                // Otherwise use vega to compute the surface triangles
                auto vegaMesh = std::static_pointer_cast<VegaVolumetricMesh>(this->meshIO->getMesh());
                ObjMesh *vegaObjMesh = GenerateSurfaceMesh::ComputeMesh(vegaMesh->getVegaMesh().get());
                const ObjMesh::Group *vegaObjMeshGroup = vegaObjMesh->getGroupHandle(0);

                // Copy triangles from vega structure...
                std::vector<std::array<size_t,3>> localTriangleArray(vegaObjMesh->getNumFaces());

                std::vector<core::Vec3d> surfaceVertices;
                for(int i = 0, end = vegaObjMeshGroup->getNumFaces(); i < end; ++i)
                {
                    localTriangleArray[i][0] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(0)->getPositionIndex();
                    localTriangleArray[i][1] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(1)->getPositionIndex();
                    localTriangleArray[i][2] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(2)->getPositionIndex();
                }
                delete vegaObjMesh;
                std::unordered_map<size_t,size_t> uniqueVertexArray;
                this->reorderSurfaceTopology(localTriangleArray,vertices,surfaceVertices,uniqueVertexArray);

                auto meshToAttach = std::make_shared<SurfaceMesh>();
                meshToAttach->setVertices(surfaceVertices);
                meshToAttach->setTriangles(localTriangleArray);
                vegaMesh->setVertexMap(uniqueVertexArray);
                vegaMesh->attachSurfaceMesh(meshToAttach);
            }
        }
        else if((this->meshProps & MeshType::Tri) && !(this->meshProps & MeshType::Tetra))
        {
            this->setSurfaceMesh(vertices,triangleArray);
        }
    }

    void copyPoints(vtkPoints *points, std::vector<core::Vec3d> &vertices) const
    {
        if(!points)
        {
            std::cerr << "VTKMeshReaderDelegate: Not points found." << std::endl;
            return;
        }
        for(vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
        {
            double position[3];
            points->GetPoint(i,position);

            vertices.emplace_back(position[0],position[1],position[2]);
        }
    }

    void copyCells(vtkCellArray *cells,
                   std::vector<std::array<size_t,3>> &triangleArray,
                   std::vector<std::array<size_t,4>> &tetraArray,
                   std::vector<std::array<size_t,8>> &hexaArray)
    {
        if(!cells)
        {
            return;
        }

        cells->InitTraversal();
        vtkNew<vtkIdList> element;
        while(cells->GetNextCell(element.GetPointer()))
        {
            switch(element->GetNumberOfIds())
            {
                case 3:
                {
                    std::array<size_t,3> e = {element->GetId(0),
                        element->GetId(1),
                        element->GetId(2)};
                    triangleArray.emplace_back(e);
                    break;
                }
                case 4:
                {
                    std::array<size_t,4> e = {element->GetId(0),
                        element->GetId(1),
                        element->GetId(2),
                        element->GetId(3)};
                    tetraArray.emplace_back(e);
                    break;
                }
                case 8:
                {
                    std::array<size_t,8> e = {element->GetId(0),
                        element->GetId(1),
                        element->GetId(2),
                        element->GetId(3),
                        element->GetId(4),
                        element->GetId(5),
                        element->GetId(6),
                        element->GetId(7)};
                    hexaArray.emplace_back(e);
                    break;
                }
            }
        }
        if(triangleArray.size() > 0)
        {
            this->meshProps |= MeshType::Tri;
        }
        if(tetraArray.size() > 0)
        {
            this->meshProps |= MeshType::Tetra;
        }
        if(hexaArray.size() > 0)
        {
            this->meshProps |= MeshType::Hexa;
        }
    }

    void copyData(vtkFieldData *fields,
                  std::vector<size_t> &constraints,
                  core::Vec3d &material)
    {
        if(!fields)
        {
            return;
        }
        // Get boundary conditions and material properties
        double mass = 0;
        double poissonRatio = 0;
        double youngModulus = 0;

        this->meshProps |= MeshType::hasMaterials;

        auto boundaryConditions = vtkUnsignedIntArray::SafeDownCast(fields->GetArray("boundary_conditions"));
        if(boundaryConditions && boundaryConditions->GetNumberOfTuples() > 0)
        {
            this->meshProps |= MeshType::hasBDConditions;
        }
        if(fields->GetArray("mass_density"))
        {
            material(0) = fields->GetArray("mass_density")->GetComponent(0,0);
        }
        if(fields->GetArray("poisson_ratio"))
        {
            material(1) = fields->GetArray("poisson_ratio")->GetComponent(0,0);
        }
        if(fields->GetArray("young_modulus"))
        {
            material(2) = fields->GetArray("young_modulus")->GetComponent(0,0);
        }
    }

    void write(){}
};

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
    SIMMEDTK_BEGIN_ONLOAD(register_VTKMeshReaderDelegate)
        SIMMEDTK_REGISTER_CLASS(IOMeshDelegate, IOMeshDelegate, VTKMeshDelegate, IOMesh::ReaderGroup::VTK);
    SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
