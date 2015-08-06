
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

#include "InputOutput/IODelegate.h"
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

class VTKMeshDelegate : IODelegate
{
public:
    void read()
    {
        auto name = this->meshIO->getFileName().c_str();
        vtkPoints *points;
        vtkCellArray *cellArray;
        vtkFieldData *fieldData;

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
                this->vtkPointsToLocal(points);
                this->vtkCellsToLocal(cellArray);
                this->vtkFieldsToLocal(fieldData);
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
                this->vtkPointsToLocal(points);
                this->vtkCellsToLocal(cellArray);
                this->vtkFieldsToLocal(fieldData);
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
                this->vtkPointsToLocal(points);
                this->vtkCellsToLocal(cellArray);
                this->vtkFieldsToLocal(fieldData);
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
                this->vtkPointsToLocal(points);
                this->vtkCellsToLocal(cellArray);
                this->vtkFieldsToLocal(fieldData);
            }
        }
    }

    void vtkPointsToLocal(vtkPoints *points)
    {
        if(!points)
        {
            std::cerr << "VTKMeshReaderDelegate: Not points found." << std::endl;
            return;
        }
        auto &vertices = this->meshIO->getMesh()->getVertices();
        for(size_t i = 0; i < points->GetNumberOfPoints(); ++i)
        {
            double position[3];
            points->GetPoint(i,position);

            vertices.emplace_back(position[0],position[1],position[2]);
        }
    }

    void vtkCellsToLocal(vtkCellArray *cells)
    {
        if(!cells)
        {
            return;
        }
        auto &triangleArray = this->meshIO->getMesh()->getTriangles();
        auto &tetraArray = this->meshIO->getMesh()->getTetrahedrons();
        auto &hexaArray = this->meshIO->getMesh()->getHexahedrons();

        cells->InitTraversal();
        vtkNew<vtkIdList> element;
        while(cells->GetNextCell(element))
        {
            switch(element->GetNumberOfIds())
            {
                case 3:
                {
                    triangleArray.emplace_back(element->GetId(0),element->GetId(1),element->GetId(2));
                    break;
                }
                case 4:
                {
                    tetraArray.emplace_back(element->GetId(0),element->GetId(1),element->GetId(2),element->GetId(3));
                    break;
                }
                case 8:
                {
                    hexaArray.emplace_back(element->GetId(0),element->GetId(1),element->GetId(2),element->GetId(3),
                                           element->GetId(4),element->GetId(5),element->GetId(6),element->GetId(7));
                    break;
                }
            }
        }
    }

    void vtkFieldsToLocal(vtkFieldData *fields)
    {
        if(!fields)
        {
            return;
        }
        // Get boundary conditions and material properties
        double mass = 0;
        double poissonRatio = 0;
        double youngModulus = 0;
        vtkUnsignedIntArray *boundaryConditions;
        if(fields)
        {
            boundaryConditions = vtkUnsignedIntArray::SafeDownCast(fields->GetArray("boundary_conditions"));
            mass = fields->GetArray("mass_density")->GetComponent(0,0);
            poissonRatio = fields->GetArray("poisson_ratio")->GetComponent(0,0);
            youngModulus = fields->GetArray("young_modulus")->GetComponent(0,0);

            std::cout << "mass: " << mass << std::endl;
            std::cout << "poissonRatio: " << poissonRatio << std::endl;
            std::cout << "youngModulus: " << youngModulus << std::endl;
        }
    }
    void write(){}
};

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
SIMMEDTK_BEGIN_ONLOAD(register_VTKMeshReaderDelegate)
SIMMEDTK_REGISTER_CLASS(IODelegate, IODelegate, VTKMeshDelegate, IOMesh::ReaderGroup::VTK);
SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
