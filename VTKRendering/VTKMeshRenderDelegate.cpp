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

#include <memory>

#include "Core/Model.h"
#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"
#include "Mesh/SurfaceMesh.h"
#include "Mesh/VegaVolumetricMesh.h"

#include "VTKRendering/MeshNodalCoordinates.h"
#include "VTKRendering/VTKRenderDelegate.h"

// VTK incudes
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkActor.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataSetMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkGeometryFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkJPEGReader.h>
#include <vtkFloatArray.h>
#include <vtkTexture.h>


class MeshRenderDelegate : public VTKRenderDelegate
{
public:
    virtual bool isTargetTextured() const override;

    vtkActor *getActor() override;
    void initDraw() override;
    void modified() override;
    void draw() const
    { }

private:
    vtkNew<vtkActor> actor;
    vtkNew<MeshNodalCoordinates<double>> mappedData;
    vtkSmartPointer<vtkMapper> mapper;

    vtkSmartPointer<vtkDataSet> dataSet;
};

void MeshRenderDelegate::initDraw()
{
    // The geometry can be either a volume or surface.
    // If its a vega volume then get its attached surface mesh.
    // This render delegate only draws surface meshes.
    auto geom = this->getSourceGeometryAs<SurfaceMesh>();
    if (!geom)
    {
        auto vega = this->getSourceGeometryAs<VegaVolumetricMesh>();
        if(!vega)
        {
            return;
        }
        auto surfaceMesh = vega->getRenderingMesh();
        if (!surfaceMesh)
        {
            return;
        }
        geom = surfaceMesh.get();
    }

    auto mesh = std::static_pointer_cast<SurfaceMesh>(geom->shared_from_this());

    mappedData->SetVertexArray(mesh->getVertices());
    vtkNew<vtkCellArray> triangles;
    auto surfaceTriangles = mesh->getTriangles();
    for(const auto &t : surfaceTriangles)
    {
        vtkIdType    cell[3];
        cell[0] = t[0];
        cell[1] = t[1];
        cell[2] = t[2];
        triangles->InsertNextCell(3,cell);
    }

    vtkNew<vtkPoints> vertices;
    vertices->SetNumberOfPoints(mesh->getVertices().size());
    vertices->SetData(mappedData.GetPointer());

    vtkNew<vtkUnstructuredGrid> unstructuredMesh;
    unstructuredMesh->SetPoints(vertices.GetPointer());
    unstructuredMesh->SetCells(VTK_TRIANGLE,triangles.GetPointer());

    vtkSmartPointer<vtkTexture> texture;
    auto renderDetail = mesh->getRenderDetail();
    if(renderDetail && renderDetail->renderTexture())
    {
        vtkNew<vtkJPEGReader> jpegReader;
        jpegReader->SetFileName(mesh->getRenderDetail()->getTextureFilename().c_str());

        texture = vtkTexture::New();
        texture->SetInputConnection(jpegReader->GetOutputPort());

        vtkNew<vtkFloatArray> textureCoordinates;
        textureCoordinates->SetNumberOfComponents(3);
        textureCoordinates->SetName("TextureCoordinates");

        auto texCoords = mesh->getTextureCoordinates();
        for(auto &coord : texCoords)
        {
            float tuple[3] = {coord[0],coord[1],0.0};
            textureCoordinates->InsertNextTuple(tuple);
        }
        unstructuredMesh->GetPointData()->SetTCoords(textureCoordinates.GetPointer());
    }

    dataSet = unstructuredMesh.GetPointer();
    if (renderDetail && renderDetail->renderNormals())
    {
        vtkSmartPointer<vtkGeometryFilter> geometry = vtkGeometryFilter::New();
        geometry->SetInputData(unstructuredMesh.GetPointer());

        vtkSmartPointer<vtkPolyDataNormals> normals = vtkPolyDataNormals::New();
        normals->SetInputConnection(geometry->GetOutputPort());
        normals->AutoOrientNormalsOn();


        mapper = vtkPolyDataMapper::New();
        mapper->SetInputConnection(normals->GetOutputPort());
    }
    else
    {
        mapper = vtkDataSetMapper::New();
        mapper->SetInputDataObject(unstructuredMesh.GetPointer());
    }

    if(texture.GetPointer())
    {
        actor->SetTexture(texture);
    }
    actor->SetMapper(mapper.GetPointer());
}

bool MeshRenderDelegate::isTargetTextured() const
{
    auto geom = this->getSourceGeometryAs<SurfaceMesh>();
    if (!geom)
        return false;

    return geom->isMeshTextured();
}

vtkActor *MeshRenderDelegate::getActor()
{
    return this->actor.GetPointer();
}

void MeshRenderDelegate::modified()
{
    if (this->dataSet)
        dataSet->Modified();
}

RegisterFactoryClass(RenderDelegate,
                     MeshRenderDelegate,
                     RenderDelegate::RendererType::VTK)
