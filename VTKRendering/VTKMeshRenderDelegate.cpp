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
#include <vtkFloatArray.h>
#include <vtkTexture.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkProperty.h>
#include "vtkShaderProgram.h"
#include "vtkOpenGLVertexArrayObject.h"
#include "vtkOpenGLVertexBufferObject.h"
#include <vtkXMLImageDataReader.h>
#include <vtkImageReader.h>
#include <vtkImageReader2Factory.h>

vtkStandardNewMacro(CustomGLPolyDataMapper)


class MeshRenderDelegate : public VTKRenderDelegate
{
public:
    virtual bool isTargetTextured() const override;

    vtkActor *getActor() override;
    void initDraw() override;
    void modified() override;
    void draw() const override
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

    auto renderDetail = mesh->getRenderDetail();
    if(renderDetail)
    {
        auto ambientColor = renderDetail->getAmbientColor().getValue();
        auto diffuseColor = renderDetail->getDiffuseColor().getValue();
        auto specularColor = renderDetail->getSpecularColor().getValue();
        auto specularPower = renderDetail->getShininess();
        auto opacity = renderDetail->getOpacity();
        actor->GetProperty()->SetAmbient(ambientColor[3]);
        actor->GetProperty()->SetAmbientColor(ambientColor[0],ambientColor[1],ambientColor[2]);
        actor->GetProperty()->SetDiffuse(diffuseColor[3]);
        actor->GetProperty()->SetDiffuseColor(diffuseColor[0],diffuseColor[1],diffuseColor[2]);
        actor->GetProperty()->SetSpecular(specularColor[3]);
        actor->GetProperty()->SetSpecularColor(specularColor[0],specularColor[1],specularColor[2]);
        actor->GetProperty()->SetSpecularPower(specularPower);
        actor->GetProperty()->SetOpacity(opacity);
    }

    if(renderDetail && renderDetail->renderWireframe())
    {
        actor->GetProperty()->SetRepresentationToWireframe();
    }

    if (renderDetail && renderDetail->renderFaces())
    {
        actor->GetProperty()->SetRepresentationToSurface();
        actor->GetProperty()->SetInterpolationToPhong();
    }

    vtkSmartPointer<vtkTexture> texture;
    if(renderDetail && renderDetail->renderTexture())
    {
		// Read texture file
		vtkSmartPointer<vtkImageReader2Factory> readerFactory =
			vtkSmartPointer<vtkImageReader2Factory>::New();
		vtkImageReader2 *imageReader =
			readerFactory->CreateImageReader2(mesh->getRenderDetail()->getTextureFilename().c_str());
		cout << mesh->getRenderDetail()->getTextureFilename() << endl;
		imageReader->SetFileName(mesh->getRenderDetail()->getTextureFilename().c_str());
		if (imageReader == NULL)
		{
			cout << "Error in opening the file" << endl;
		
		}
		imageReader->Update();


        texture = vtkTexture::New();
      	texture->SetInputConnection(imageReader->GetOutputPort());

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
	//vtkNew<vtkDoubleArray> tangentArray;
	//tangentArray->SetNumberOfComponents(3);
	//tangentArray->SetName("Tangents");

	//vtkNew<vtkPoints> tangents;
	//tangents->SetNumberOfPoints(mesh->getVertices().size());
	
	
    dataSet = unstructuredMesh.GetPointer();
    if (renderDetail && renderDetail->renderNormals())
    {
        vtkSmartPointer<vtkGeometryFilter> geometry = vtkGeometryFilter::New();
        geometry->SetInputData(unstructuredMesh.GetPointer());

        vtkSmartPointer<vtkPolyDataNormals> normals = vtkPolyDataNormals::New();
        normals->SetInputConnection(geometry->GetOutputPort());
        normals->AutoOrientNormalsOn();

		mapper = CustomGLPolyDataMapper::New();
        mapper->SetInputConnection(normals->GetOutputPort());
		auto mapperCustom = CustomGLPolyDataMapper::SafeDownCast(mapper);
		mapperCustom->renderDetail = renderDetail;
		mesh->computeVertexNeighbors();
		mesh->setUseOBJTexture(true);
		mesh->computeTriangleTangents();
		
		mapperCustom->tangents = mesh->getVertexTangents();
		
        if(renderDetail->hasShaders())
        {
            auto glMapper = vtkOpenGLPolyDataMapper::SafeDownCast(mapper);
            auto shadersPrograms = renderDetail->getShaderPrograms();
            this->setShadersProgram(glMapper,shadersPrograms);
		

            auto shadersProgramReplacements = renderDetail->getShaderProgramReplacements();
            //this->setShadersProgramReplacements(glMapper,shadersProgramReplacements);
			
			
			
        }
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

void CustomGLPolyDataMapper::initDraw(){


	cout << "init" << endl;

}
void CustomGLPolyDataMapper::BuildBufferObjects(vtkRenderer *ren, vtkActor *act){
	tangentsBuffer = vtkOpenGLBufferObject::New();
	tangentsBuffer->Bind();
	if (tangentsBuffer->Upload(this->tangents, vtkOpenGLBufferObject::ArrayBuffer)){
		cout << "Completed Tangents Binding"<<endl;
	
	
	}
	tangentsBuffer->Release();
	vtkOpenGLPolyDataMapper::BuildBufferObjects(ren,act);

}

void CustomGLPolyDataMapper::SetMapperShaderParameters(vtkOpenGLHelper &cellBO, vtkRenderer *ren, vtkActor *act)
{
	double testColor1 = 0.01;
	double testColor2 = 0.01;
	static double testColor3 = 0.01;
	vtkShaderProgram *program = cellBO.Program;
	
	testColor3 += 0.01;
	program->SetUniformf("TestColor1", testColor1);
	program->SetUniformf("TestColor2", testColor2);
	program->SetUniformf("TestColor3", testColor3);
	cellBO.VAO->Bind();
	
	if (!cellBO.VAO->AddAttributeArray(cellBO.Program, this->tangentsBuffer,
		"vertTangents", 0,
		0, VTK_DOUBLE, 3, false))
	{
		vtkErrorMacro(<< "Error setting 'vertTangents' in shader VAO.");
	}
	vtkOpenGLPolyDataMapper::SetMapperShaderParameters(cellBO, ren, act);
		
}

RegisterFactoryClass(RenderDelegate,
                     MeshRenderDelegate,
                     RenderDelegate::RendererType::VTK)
