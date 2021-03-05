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

#include "imstkVTKSurfaceMeshRenderDelegate.h"
#include "imstkGeometryUtilities.h"
#include "imstkRenderMaterial.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"

#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLVertexBufferObject.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkVersion.h>

namespace imstk
{
VTKSurfaceMeshRenderDelegate::VTKSurfaceMeshRenderDelegate(std::shared_ptr<VisualModel> visualModel) : VTKPolyDataRenderDelegate(visualModel),
    m_polydata(vtkSmartPointer<vtkPolyData>::New()),
    m_mappedVertexArray(vtkSmartPointer<vtkDoubleArray>::New()),
    m_mappedNormalArray(vtkSmartPointer<vtkDoubleArray>::New())
{
    m_geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());
    m_geometry->computeVertexNeighborTriangles();

    // Get our own handles to these in case the geometry changes them
    m_vertices = m_geometry->getVertexPositions();
    m_indices  = m_geometry->getTriangleIndices();

    // Map vertices to VTK point data
    if (m_vertices != nullptr)
    {
        m_mappedVertexArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_vertices));
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(m_geometry->getNumVertices());
        points->SetData(m_mappedVertexArray);
        m_polydata->SetPoints(points);
    }

    // Map indices to VTK cell data (copied)
    if (m_indices != nullptr)
    {
        m_cellArray = vtkSmartPointer<vtkCellArray>::New();
        vtkIdType cell[3];
        for (const auto& t : *m_indices)
        {
            for (size_t i = 0; i < 3; ++i)
            {
                cell[i] = t[i];
            }
            m_cellArray->InsertNextCell(3, cell);
        }
        m_polydata->SetPolys(m_cellArray);
    }

    // Map vertex scalars if it has them
    if (m_geometry->getVertexScalars() != nullptr)
    {
        m_mappedVertexScalarArray = GeometryUtils::coupleVtkDataArray(m_geometry->getVertexScalars());
        m_polydata->GetPointData()->SetScalars(m_mappedVertexScalarArray);
    }

    // Map cell scalars if it has them
    if (m_geometry->getCellScalars() != nullptr)
    {
        m_mappedCellScalarArray = GeometryUtils::coupleVtkDataArray(m_geometry->getCellScalars());
        m_polydata->GetCellData()->SetScalars(m_mappedCellScalarArray);
    }

    // Map normals, if none provided compute per vertex normals
    if (m_geometry->getVertexNormals() == nullptr)
    {
        m_geometry->computeVertexNormals();
    }
    m_mappedNormalArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_geometry->getVertexNormals()));
    m_polydata->GetPointData()->SetNormals(m_mappedNormalArray);

    // Map TCoords
    if (m_geometry->getVertexTCoords() != nullptr)
    {
        m_mappedTCoordsArray = vtkFloatArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_geometry->getVertexTCoords()));
        m_mappedTCoordsArray->SetName(m_geometry->getActiveVertexTCoords().c_str());
        m_polydata->GetPointData()->SetTCoords(m_mappedTCoordsArray);

        // Map Tangents
        //geometry->computeVertexTangents();
        //if (geometry->getVertexTangents() != nullptr)
        //{
        //    // todo: I might need these as float for PBR?
        //    m_mappedTangentArray = vtkDoubleArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(geometry->getVertexTangents()));
        //    m_polydata->GetPointData()->SetTangents(m_mappedTangentArray);
        //}
    }

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(m_geometry, EventType::Modified, this, &VTKSurfaceMeshRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(m_geometry->getVertexPositions(), EventType::Modified, this, &VTKSurfaceMeshRenderDelegate::vertexDataModified);

    // When the index buffer internals are modified,

    // Setup mapper
    {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputData(m_polydata);
        vtkNew<vtkActor> actor;
        actor->SetMapper(mapper);
        //actor->SetUserTransform(m_transform);
        m_mapper = mapper;
        m_actor  = actor;
        if (auto glMapper = vtkOpenGLPolyDataMapper::SafeDownCast(m_mapper))
        {
            glMapper->SetVBOShiftScaleMethod(vtkOpenGLVertexBufferObject::DISABLE_SHIFT_SCALE);
        }
    }

    update();
    updateRenderProperties();
}

void
VTKSurfaceMeshRenderDelegate::processEvents()
{
    // Custom handling of events
    std::shared_ptr<SurfaceMesh>             geom     = std::dynamic_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());
    std::shared_ptr<VecDataArray<double, 3>> vertices = geom->getVertexPositions();

    // Only use the most recent event from respective sender
    std::list<Command> cmds;
    bool               contains[4] = { false, false, false, false };
    rforeachEvent([&](Command cmd)
        {
            if (cmd.m_event->m_sender == m_visualModel.get() && !contains[0])
            {
                cmds.push_back(cmd);
                contains[0] = true;
            }
            else if (cmd.m_event->m_sender == m_material.get() && !contains[1])
            {
                cmds.push_back(cmd);
                contains[1] = true;
            }
            else if (cmd.m_event->m_sender == geom.get() && !contains[2])
            {
                cmds.push_back(cmd);
                contains[2] = true;
            }
            else if (cmd.m_event->m_sender == vertices.get() && !contains[3])
            {
                cmds.push_back(cmd);
                contains[3] = true;
            }
        });

    // Now do each event in order recieved
    for (std::list<Command>::reverse_iterator i = cmds.rbegin(); i != cmds.rend(); i++)
    {
        i->invoke();
    }
}

void
VTKSurfaceMeshRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());

    // Update the pointer of the coupled array
    m_vertices = geometry->getVertexPositions();
    if (m_vertices->getVoidPointer() != m_mappedVertexArray->GetVoidPointer(0))
    {
        m_mappedVertexArray->SetNumberOfComponents(3);
        m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    }
    m_mappedVertexArray->Modified();

    // If the material says we should recompute normals
    if (m_visualModel->getRenderMaterial()->getRecomputeVertexNormals())
    {
        geometry->computeVertexNormals();
        std::shared_ptr<VecDataArray<double, 3>> normals    = geometry->getVertexNormals();
        double*                                  normalData = reinterpret_cast<double*>(normals->getPointer());
        m_mappedNormalArray->SetNumberOfComponents(3);
        m_mappedNormalArray->SetArray(normalData, normals->size() * 3, 1);
        m_mappedNormalArray->Modified();
    }
}

//void
//VTKSurfaceMeshRenderDelegate::indexDataModified(Event* e)
//{
//
//}

void
VTKSurfaceMeshRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    auto geometry = std::static_pointer_cast<SurfaceMesh>(m_visualModel->getGeometry());

    //bool vertexOrIndexBufferChanged = false;

    // If the vertices were reallocated
    if (m_vertices != geometry->getVertexPositions())
    {
        //printf("Vertex data swapped\n");
        m_vertices = geometry->getVertexPositions();
        {
            // Update the pointer of the coupled array
            m_mappedVertexArray->SetNumberOfComponents(3);
            m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
        }
        m_polydata->GetPoints()->SetNumberOfPoints(m_vertices->size());
        //vertexOrIndexBufferChanged = true;
    }

    // Notify VTK that the vertices were changed
    m_mappedVertexArray->Modified();

    // Only update index buffer when reallocated
    if (m_indices != geometry->getTriangleIndices())
    {
        m_indices = geometry->getTriangleIndices();
        {
            // Copy cells
            m_cellArray->Reset();
            vtkIdType cell[3];
            for (const auto& t : *m_indices)
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    cell[i] = t[i];
                }
                m_cellArray->InsertNextCell(3, cell);
            }
            m_cellArray->Modified();
        }
    }

    if (m_visualModel->getRenderMaterial()->getRecomputeVertexNormals())
    {
        geometry->computeVertexNormals();
        std::shared_ptr<VecDataArray<double, 3>> normals    = geometry->getVertexNormals();
        double*                                  normalData = reinterpret_cast<double*>(normals->getPointer());
        m_mappedNormalArray->SetNumberOfComponents(3);
        m_mappedNormalArray->SetArray(normalData, normals->size() * 3, 1);
        m_mappedNormalArray->Modified();
    }
}

void
VTKSurfaceMeshRenderDelegate::initializeTextures(TextureManager<VTKTextureDelegate>& textureManager)
{
    auto material = m_visualModel->getRenderMaterial();
    if (material == nullptr)
    {
        return;
    }

    unsigned int currentUnit = 0;

    // Go through all of the textures
    for (int unit = 0; unit < (int)Texture::Type::None; unit++)
    {
        // Get imstk texture
        auto texture = material->getTexture((Texture::Type)unit);
        if (std::strcmp(texture->getPath().c_str(), "") == 0)
        {
            continue;
        }

        // Get vtk texture
        auto textureDelegate = textureManager.getTextureDelegate(texture);

        /* /!\ VTKTextureWrapMode not yet supported in VTK 7
        * See here for some work that needs to be imported back to upstream:
        * https://gitlab.kitware.com/iMSTK/vtk/commit/62a7ecd8a5f54e243c26960de22d5d1d23ef932b
        *
        texture->SetWrapMode(vtkTexture::VTKTextureWrapMode::ClampToBorder);

        * /!\ MultiTextureAttribute not yet supported in VTK 7
        * See here for some work that needs to be imported back to upstream:
        * https://gitlab.kitware.com/iMSTK/vtk/commit/ae373026755db42b6fdce5093109ef1a39a76340
        *
        // Link texture unit to texture attribute
        m_mapper->MapDataArrayToMultiTextureAttribute(unit, tCoordsName.c_str(),
                                                    vtkDataObject::FIELD_ASSOCIATION_POINTS);
        */

        // Set texture
        auto currentTexture = textureDelegate->getTexture();

        vtkSmartPointer<vtkActor> actor = vtkActor::SafeDownCast(m_actor);

#if (VTK_MAJOR_VERSION <= 8 && VTK_MINOR_VERSION <= 1)
        actor->GetProperty()->SetTexture(currentUnit, currentTexture);
#else
        if (material->getShadingModel() == RenderMaterial::ShadingModel::PBR)
        {
            switch (texture->getType())
            {
            case Texture::Type::Diffuse:
            {
                actor->GetProperty()->SetBaseColorTexture(currentTexture);
                break;
            }
            case Texture::Type::Normal:
            {
                actor->GetProperty()->SetNormalTexture(currentTexture);
                actor->GetProperty()->SetNormalScale(material->getNormalStrength());
                break;
            }
            case Texture::Type::AmbientOcclusion:
            {
                actor->GetProperty()->SetORMTexture(currentTexture);
                actor->GetProperty()->SetOcclusionStrength(material->getOcclusionStrength());
                break;
            }
            default:
            {
            }
            }
        }
        else
        {
            actor->GetProperty()->SetTexture(textureDelegate->getTextureName().c_str(), currentTexture);
        }

#endif

        currentUnit++;
    }
}
} // imstk
