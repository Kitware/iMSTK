
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
#include <vtkXMLGenericDataObjectReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedIntArray.h>
#include <vtkCellArray.h>
#include <vtkFieldData.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkSTLReader.h>

///
/// \brief This delegate implements the VTK based readers/writers. It creates
/// a VegaVolumetric mesh for tetra/hexaarrays and a surface mesh otherwise.
///
class IOMeshVTKDelegate : public IOMeshDelegate
{
public:
    ///
    /// \brief Performs the actual reading of the mesh and
    ///
    void read()
    {
        // Get file name and define some variables.
        auto name = this->meshIO->getFileName().c_str();
        std::vector<core::Vec3d> vertices;
        std::vector<std::array<size_t,3>> triangleArray;
        std::vector<std::array<size_t,4>> tetraArray;
        std::vector<std::array<size_t,8>> hexaArray;
        std::vector<size_t> bdConditions;
        core::Vec3d materials;

        // Record the type of mesh we have.
        this->meshProps = 0;

        // Choose the reader according to filetype
        switch(this->meshIO->getFileType())
        {
            // VTK obj reader
            case IOMesh::MeshFileType::OBJ:
            {
                vtkNew<vtkOBJReader> reader;
                reader->SetFileName(name);
                reader->Update();

                // Copy data to local arrays
                this->copyPoints(reader->GetOutput()->GetPoints(),vertices);
                this->copyCells(reader->GetOutput()->GetPolys(),triangleArray,tetraArray,hexaArray);
                this->copyData(reader->GetOutput()->GetFieldData(),bdConditions,materials);
                this->meshProps |= MeshType::Tri;
                break;
            }
            // VTK STL reader
            case IOMesh::MeshFileType::STL:
            {
                vtkNew<vtkSTLReader> reader;
                reader->SetFileName(name);
                reader->Update();

                // Copy data to local arrays
                this->copyPoints(reader->GetOutput()->GetPoints(),vertices);
                this->copyCells(reader->GetOutput()->GetPolys(),triangleArray,tetraArray,hexaArray);
                this->copyData(reader->GetOutput()->GetFieldData(),bdConditions,materials);
                this->meshProps |= MeshType::Tri;
                break;
            }
            // VTK PLY reader
            case IOMesh::MeshFileType::PLY:
            {
                vtkNew<vtkPLYReader> reader;
                reader->SetFileName(name);
                reader->Update();

                // Copy data to local arrays
                this->copyPoints(reader->GetOutput()->GetPoints(),vertices);
                this->copyCells(reader->GetOutput()->GetPolys(),triangleArray,tetraArray,hexaArray);
                this->copyData(reader->GetOutput()->GetFieldData(),bdConditions,materials);
                this->meshProps |= MeshType::Tri;
                break;
            }
            // VTK legacy formats
            case IOMesh::MeshFileType::VTK:
            {
                this->readGenericFormat<vtkGenericDataObjectReader>(
                    name,
                    vertices,
                    triangleArray,
                    tetraArray,
                    hexaArray,
                    bdConditions,
                    materials
                );
                break;
            }
            // VTK VTU formats
            case IOMesh::MeshFileType::VTU:
            {
                this->readGenericFormat<vtkXMLGenericDataObjectReader>(
                    name,
                    vertices,
                    triangleArray,
                    tetraArray,
                    hexaArray,
                    bdConditions,
                    materials
                );
                break;
            }
            // VTK VTU formats
            case IOMesh::MeshFileType::VTP:
            {
                this->readGenericFormat<vtkXMLGenericDataObjectReader>(
                    name,
                    vertices,
                    triangleArray,
                    tetraArray,
                    hexaArray,
                    bdConditions,
                    materials
                );
                break;
            }
            default:
            {
                std::cerr << "IOMeshVTKDelegate: Unsupported file format." << std::endl;
                return;
            }
        }

        // If the mesh has tetrahedral elements as well as triangle
        // elements we assume that the triangles correspond
        // to the triangles on the surface of the volume mesh. If
        // the mesh has only tetras, then create the surface
        // elements and store it in a vega volumetric mesh.
        if(this->meshProps & MeshType::Tetra)
        {
            if((this->meshProps & MeshType::hasBDConditions) &&
                (this->meshProps & MeshType::hasMaterials))
            {
                this->setVegaTetraMesh(vertices,tetraArray,bdConditions,materials);
            }
            else
            {
                this->setVegaTetraMesh(vertices,tetraArray);
            }

            // If the vtk dataset also contains surface triangles, then
            // store them in a SurfaceMesh and attach to the volumetric mesh.
            // Otherwise use vega to compute the surface triangles.
            if(this->meshProps & MeshType::Tri)
            {
                std::vector<core::Vec3d> surfaceVertices;
                std::unordered_map<size_t,size_t> uniqueVertexArray;
                this->reorderSurfaceTopology(
                    vertices,
                    surfaceVertices,
                    triangleArray,
                    uniqueVertexArray);

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
                auto vegaMesh = std::static_pointer_cast<VegaVolumetricMesh>(this->meshIO->getMesh());
                ObjMesh *vegaObjMesh = GenerateSurfaceMesh::ComputeMesh(vegaMesh->getVegaMesh().get());
                const ObjMesh::Group *vegaObjMeshGroup = vegaObjMesh->getGroupHandle(0);

                // Copy triangles from vega surface mesh.
                triangleArray.resize(vegaObjMesh->getNumFaces());

                for(int i = 0, end = vegaObjMeshGroup->getNumFaces(); i < end; ++i)
                {
                    triangleArray[i][0] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(0)->getPositionIndex();
                    triangleArray[i][1] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(1)->getPositionIndex();
                    triangleArray[i][2] = vegaObjMeshGroup->getFaceHandle(i)->getVertexHandle(2)->getPositionIndex();
                }
                delete vegaObjMesh;
                std::vector<core::Vec3d> surfaceVertices;
                std::unordered_map<size_t,size_t> uniqueVertexArray;
                this->reorderSurfaceTopology(
                    vertices,
                    surfaceVertices,
                    triangleArray,
                    uniqueVertexArray);

                auto meshToAttach = std::make_shared<SurfaceMesh>();
                meshToAttach->setVertices(surfaceVertices);
                meshToAttach->setTriangles(triangleArray);
                vegaMesh->setVertexMap(uniqueVertexArray);
                vegaMesh->attachSurfaceMesh(meshToAttach);
            }
        }
        else if((this->meshProps & MeshType::Tri) && !(this->meshProps & MeshType::Tetra))
        {
            this->setSurfaceMesh(vertices,triangleArray);
        }
    }

    ///
    /// \brief Copy vtk points into vertices.
    /// \param points VTK point taken from the dataset.
    /// \param vertices Array oof vertices holding coordinates.
    ///
    void copyPoints(vtkPoints *points, std::vector<core::Vec3d> &vertices) const
    {
        if(!points)
        {
            std::cerr << "IOMeshVTKDelegate: Not points found." << std::endl;
            return;
        }
        for(vtkIdType i = 0, end = points->GetNumberOfPoints(); i < end; ++i)
        {
            double position[3];
            points->GetPoint(i,position);

            vertices.emplace_back(position[0],position[1],position[2]);
        }
    }

    ///
    /// \brief Copy vtk cells into triangles.
    /// \param cells VTK cell array taken from the dataset.
    /// \param tetraArray Array of triangles holding indices to vertices.
    /// \param tetraArray Array of tetrahedrons holding indices to vertices.
    /// \param hexaArray Array of hexahedrons holding indices to vertices.
    ///
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

    ///
    /// \brief Copy vtk field data.
    /// \param cells VTK fields array taken from the dataset.
    /// \param constraints Output array of a vector holding indices to vertices.
    /// \param material Array of material properties.
    ///
    void copyData(vtkFieldData *fields,
                  std::vector<size_t> &constraints,
                  core::Vec3d &material)
    {
        if(!fields)
        {
            return;
        }
        // Get boundary conditions and material properties

        this->meshProps |= MeshType::hasMaterials;

        auto boundaryConditions = vtkUnsignedIntArray::SafeDownCast(fields->GetArray("boundary_conditions"));
        if(boundaryConditions && boundaryConditions->GetNumberOfTuples() > 0)
        {
            constraints.resize(boundaryConditions->GetNumberOfTuples());
            for(size_t i = 0; i < boundaryConditions->GetNumberOfTuples(); ++i)
            {
                constraints[i] = boundaryConditions->GetValue(i);
            }
            this->meshProps |= MeshType::hasBDConditions;
        }
        if(fields->GetArray("mass_density"))
        {
            material(0) = fields->GetArray("mass_density")->GetComponent(0,0);
            this->meshProps |= MeshType::hasDensity;
        }
        if(fields->GetArray("poisson_ratio"))
        {
            material(1) = fields->GetArray("poisson_ratio")->GetComponent(0,0);
            this->meshProps |= MeshType::hasPoisson;
        }
        if(fields->GetArray("young_modulus"))
        {
            material(2) = fields->GetArray("young_modulus")->GetComponent(0,0);
            this->meshProps |= MeshType::hasYoung;
        }
    }

    void write(){}

    template<typename GenericReader>
    void readGenericFormat(const std::string &name,
                        std::vector<core::Vec3d> &vertices,
                        std::vector<std::array<size_t,3>> &triangleArray,
                        std::vector<std::array<size_t,4>> &tetraArray,
                        std::vector<std::array<size_t,8>> &hexaArray,
                        std::vector<size_t> &bdConditions,
                        core::Vec3d &materials)
    {
        vtkNew<GenericReader> reader;
        reader->SetFileName(name.c_str());
        reader->Update();

        vtkPointSet *output = vtkPointSet::SafeDownCast(reader->GetOutput());

        if(!output)
        {
            std::cerr << "IOMeshVTKDelegate: Empty reader for vtk files." << std::endl;
            return;
        }

        // Copy data to local arrays
        this->copyPoints(output->GetPoints(),vertices);
        this->copyData(output->GetFieldData(),bdConditions,materials);
        if(auto out = reader->GetPolyDataOutput())
        {
            this->copyCells(out->GetPolys(),triangleArray,tetraArray,hexaArray);
        }
        else if(auto out = reader->GetUnstructuredGridOutput())
        {
            this->copyCells(out->GetCells(),triangleArray,tetraArray,hexaArray);
        }
        else
        {
            std::cerr << "IOMeshVTKDelegate: Unsupported dataset." << std::endl;
        }
    }

};

RegisterFactoryClass(IOMeshDelegate,IOMeshVTKDelegate,IOMesh::ReaderGroup::VTK)
