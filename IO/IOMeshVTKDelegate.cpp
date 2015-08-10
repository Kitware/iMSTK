
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

#include "IO/IOMeshDelegate.h"
#include "Core/Factory.h"
#include "Mesh/SurfaceMesh.h"
#include "Mesh/VegaVolumetricMesh.h"

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
    enum MeshType 
    {
        None    = 0,
        Tri     = 1 << 0,
        Tetra   = 1 << 1,
        Hexa    = 1 << 2,
        hasMatrials = 1 << 3,
        hasBDConditions = 1 << 4
        
    };
    int meshProps;
    
public:
    void read() const
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
            if((this->meshProps & MeshType::hasBDConditions) && (this->meshProps & MeshType::hasMatrials))
            {
                this->setVegaTetraMesh(vertices,tetraArray,bdConditions,materials);
            }
            else
            {
                this->setVegaTetraMesh(vertices,tetraArray);
            }
            if(this->meshProps & MeshType::Tri)
            {
                auto surfaceMesh = std::make_shared<SurfaceMesh>();
                std::vector<core::Vec3d> surfaceVertices;
                for(auto &t: triangleArray)
                {
                    surfaceVertices.push_back(vertexArray[t(0)]);
                    surfaceVertices.push_back(vertexArray[t(1)]);
                    surfaceVertices.push_back(vertexArray[t(2)]);
                }
                surfaceMesh->setVertices(surfaceVertices);
                surfaceMesh->setTriangles(triangleArray);
                std::static_pointer_cast<VegaVolumetricMesh>(this->meshIO->getMesh())->attachSurfaceMesh(surfaceMesh,2.0);
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

    bool copyCells(vtkCellArray *cells, std::vector<std::array<size_t,3>> &triangleArray, std::vector<std::array<size_t,4>> &tetraArray, std::vector<std::array<size_t,8>> &hexaArray) const
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
                    std::array<size_t,3> e = {element->GetId(0),element->GetId(1),element->GetId(2)};
                    triangleArray.emplace_back(e);
                    break;
                }
                case 4:
                {
                    std::array<size_t,4> e = {element->GetId(0),element->GetId(1),element->GetId(2),element->GetId(3)};
                    tetraArray.emplace_back(e);
                    break;
                }
                case 8:
                {
                    std::array<size_t,8> e = {element->GetId(0),element->GetId(1),element->GetId(2),element->GetId(3),
                                              element->GetId(4),element->GetId(5),element->GetId(6),element->GetId(7)};
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

    void copyData(vtkFieldData *fields, std::vector<size_t> &constraints, core::Vec3d &material) const
    {
        if(!fields)
        {
            return;
        }
        // Get boundary conditions and material properties
        double mass = 0;
        double poissonRatio = 0;
        double youngModulus = 0;

        this->meshProps |= MeshType::hasMatrials;
        
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
    
    void setSurfaceMesh(const std::vector<core::Vec3d> &vertices, 
                        const std::vector<std::array<size_t,3>> &triangleArray)
    {
        auto mesh = std::make_shared<SurfaceMesh>();
        mesh->setVertices(vertices);
        mesh->setTriangles(triangleArray);
        this->meshIO->setMesh(mesh);
    }
    
    void setVegaTetraMesh(const std::vector<core::Vec3d> &vertices, 
                          const std::vector<std::array<size_t,4>> &tetraArray, 
                          const std::vector<size_t> &bdConditions, 
                          const core::Vec3d &material)
    {
        auto tetraMesh = std::make_shared<VegaVolumetricMesh>();
        auto vegaMesh = std::make_shared<TetMesh>(vertices.size(),
                                                  vertices.data().data(),
                                                  tetraArray.size(),
                                                  tetraArray.data().data(),
                                                  material[0],
                                                  material[1],
                                                  material[2]);
        tetraMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(tetraMesh);        
    }
    
    void setVegaTetraMesh(const std::vector<core::Vec3d> &vertices, 
                          const std::vector<std::array<size_t,4>> &tetraArray)
    {
        auto tetraMesh = std::make_shared<VegaVolumetricMesh>();
        auto vegaMesh = std::make_shared<TetMesh>(vertices.size(),
                                                  vertices.data().data(),
                                                  tetraArray.size(),
                                                  tetraArray.data().data());
        tetraMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(tetraMesh);        
    }    
    
    void setVegaHexaMesh(const std::vector<core::Vec3d> &vertices, 
                         const std::vector<std::array<size_t,8>> &hexaArray, 
                         const std::vector<size_t> &bdConditions, 
                         const std::array<double,3> &material)
    {
        auto hexaMesh = std::make_shared<VegaVolumetricMesh>();
        auto vegaMesh = std::make_shared<CubicMesh>(vertices.size(),
                                                    vertices.data().data(),
                                                    hexaArray.size(),
                                                    hexaArray.data().data(),
                                                    material[0],
                                                    material[1],
                                                    material[2]);
        hexaMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(hexaMesh);
    }
    
    void setVegaHexaMesh(const std::vector<core::Vec3d> &vertices, 
                         const std::vector<std::array<size_t,8>> &hexaArray)
    {
        auto hexaMesh = std::make_shared<VegaVolumetricMesh>();
        auto vegaMesh = std::make_shared<CubicMesh>(vertices.size(),
                                                    vertices.data().data(),
                                                    hexaArray.size(),
                                                    hexaArray.data().data());
        hexaMesh->setVegaMesh(vegaMesh);
        this->meshIO->setMesh(hexaMesh);
    }    
    void write(){}
};

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
SIMMEDTK_BEGIN_ONLOAD(register_VTKMeshReaderDelegate)
SIMMEDTK_REGISTER_CLASS(IOMeshDelegate, IOMeshDelegate, VTKMeshDelegate, IOMesh::ReaderGroup::VTK);
SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
