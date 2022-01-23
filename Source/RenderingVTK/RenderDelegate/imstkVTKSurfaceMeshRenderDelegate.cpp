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
#include "imstkTextureDelegate.h"
#include "imstkTextureManager.h"
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
            for (size_t i = 0; i < 3; i++)
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
        setVertexScalarBuffer(m_geometry->getVertexScalars());
    }

    // Map cell scalars if it has them
    if (m_geometry->getCellScalars() != nullptr)
    {
        setCellScalarBuffer(m_geometry->getCellScalars());
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
        setTextureCoordinateBuffer(m_geometry->getVertexTCoords());
    }

    // When geometry is modified, update data source, mostly for when an entirely new array/buffer was set
    queueConnect<Event>(m_geometry, &Geometry::modified, this, &VTKSurfaceMeshRenderDelegate::geometryModified);

    // When the vertex buffer internals are modified, ie: a single or N elements
    queueConnect<Event>(m_geometry->getVertexPositions(), &VecDataArray<double, 3>::modified, this, &VTKSurfaceMeshRenderDelegate::vertexDataModified);

    // When index buffer internals are modified
    queueConnect<Event>(m_geometry->getTriangleIndices(), &VecDataArray<int, 3>::modified, this, &VTKSurfaceMeshRenderDelegate::indexDataModified);

    // When index buffer internals are modified
    queueConnect<Event>(m_geometry->getVertexNormals(), &VecDataArray<double, 3>::modified, this, &VTKSurfaceMeshRenderDelegate::normalDataModified);

    connect<Event>(m_material, &RenderMaterial::texturesModified, this, &VTKSurfaceMeshRenderDelegate::texturesModified);

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
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr           = m_geometry->getVertexPositions();
    std::shared_ptr<VecDataArray<int, 3>>    indicesPtr            = m_geometry->getTriangleIndices();
    std::shared_ptr<AbstractDataArray>       cellScalarsPtr        = m_geometry->getCellScalars();
    std::shared_ptr<AbstractDataArray>       vertexScalarsPtr      = m_geometry->getVertexScalars();
    std::shared_ptr<AbstractDataArray>       textureCoordinatesPtr = m_geometry->getVertexTCoords();

    // Only use the most recent event from respective sender
    std::array<Command, 8> cmds;
    std::array<bool, 8>    contains = { false, false, false, false, false, false, false, false };
    rforeachEvent([&](Command cmd)
        {
            if (cmd.m_event->m_sender == m_visualModel.get() && !contains[0])
            {
                cmds[0]     = cmd;
                contains[0] = true;
            }
            else if (cmd.m_event->m_sender == m_material.get() && !contains[1])
            {
                cmds[1]     = cmd;
                contains[1] = true;
            }
            else if (cmd.m_event->m_sender == m_geometry.get() && !contains[2])
            {
                cmds[2]     = cmd;
                contains[2] = true;
            }
            else if (cmd.m_event->m_sender == verticesPtr.get() && !contains[3])
            {
                cmds[3]     = cmd;
                contains[3] = true;
            }
            else if (cmd.m_event->m_sender == cellScalarsPtr.get() && !contains[4])
            {
                cmds[4]     = cmd;
                contains[4] = true;
            }
            else if (cmd.m_event->m_sender == vertexScalarsPtr.get() && !contains[5])
            {
                cmds[5]     = cmd;
                contains[5] = true;
            }
            else if (cmd.m_event->m_sender == indicesPtr.get() && !contains[6])
            {
                cmds[6]     = cmd;
                contains[6] = true;
            }
            else if (cmd.m_event->m_sender == textureCoordinatesPtr.get() && !contains[7])
            {
                cmds[7]     = cmd;
                contains[7] = true;
            }
        });

    // Now do all the commands
    cmds[0].invoke(); // Update VisualModel
    cmds[1].invoke(); // Update RenderMaterial
    cmds[3].invoke(); // Update vertices
    cmds[4].invoke(); // Update cell scalars
    cmds[5].invoke(); // Update vertex scalars
    cmds[6].invoke(); // Update indices
    cmds[7].invoke(); // Update texture coordinates
    cmds[2].invoke(); // Update geometry as a whole
}

void
VTKSurfaceMeshRenderDelegate::vertexDataModified(Event* imstkNotUsed(e))
{
    setVertexBuffer(m_geometry->getVertexPositions());

    // If the material says we should recompute normals
    if (m_visualModel->getRenderMaterial()->getRecomputeVertexNormals())
    {
        m_geometry->computeVertexNormals();
        setNormalBuffer(m_geometry->getVertexNormals());
    }
}

void
VTKSurfaceMeshRenderDelegate::indexDataModified(Event* imstkNotUsed(e))
{
    setIndexBuffer(m_geometry->getTriangleIndices());
}

void
VTKSurfaceMeshRenderDelegate::normalDataModified(Event* imstkNotUsed(e))
{
    setNormalBuffer(m_geometry->getVertexNormals());
}

void
VTKSurfaceMeshRenderDelegate::vertexScalarsModified(Event* imstkNotUsed(e))
{
    setVertexScalarBuffer(m_geometry->getVertexScalars());
}

void
VTKSurfaceMeshRenderDelegate::cellScalarsModified(Event* imstkNotUsed(e))
{
    setCellScalarBuffer(m_geometry->getCellScalars());
}

void
VTKSurfaceMeshRenderDelegate::textureCoordinatesModified(Event* imstkNotUsed(e))
{
    setTextureCoordinateBuffer(m_geometry->getVertexTCoords());
}

void
VTKSurfaceMeshRenderDelegate::geometryModified(Event* imstkNotUsed(e))
{
    // If the vertices were reallocated
    if (m_vertices != m_geometry->getVertexPositions())
    {
        setVertexBuffer(m_geometry->getVertexPositions());
    }

    // Assume vertices are always changed
    m_mappedVertexArray->Modified();

    // Only update index buffer when reallocated
    if (m_indices != m_geometry->getTriangleIndices())
    {
        setIndexBuffer(m_geometry->getTriangleIndices());
    }

    if (m_normals != m_geometry->getVertexNormals())
    {
        setNormalBuffer(m_geometry->getVertexNormals());
    }

    if (m_visualModel->getRenderMaterial()->getRecomputeVertexNormals())
    {
        m_geometry->computeVertexNormals();
        setNormalBuffer(m_geometry->getVertexNormals());
    }

    if (m_vertexScalars != m_geometry->getVertexScalars())
    {
        setVertexScalarBuffer(m_geometry->getVertexScalars());
    }

    if (m_cellScalars != m_geometry->getCellScalars())
    {
        setCellScalarBuffer(m_geometry->getCellScalars());
    }

    if (m_textureCoordinates != m_geometry->getVertexTCoords())
    {
        setTextureCoordinateBuffer(m_geometry->getVertexTCoords());
    }
}

void
VTKSurfaceMeshRenderDelegate::texturesModified(Event* e)
{
    // If a texture is set/swapped reinit all textures
    RenderMaterial* material = static_cast<RenderMaterial*>(e->m_sender);
    if (material != nullptr)
    {
        // Reload all textures
        // If texture already present, don't do anything unless name changed
        initializeTextures();
    }
}

void
VTKSurfaceMeshRenderDelegate::setVertexBuffer(std::shared_ptr<VecDataArray<double, 3>> vertices)
{
    // If the buffer changed
    if (m_vertices != vertices)
    {
        // If previous buffer exist
        if (m_vertices != nullptr)
        {
            // stop observing its changes
            disconnect(m_vertices, this, &VecDataArray<double, 3>::modified);
        }
        // Set new buffer and observe
        m_vertices = vertices;
        queueConnect<Event>(m_vertices, &VecDataArray<double, 3>::modified, this, &VTKSurfaceMeshRenderDelegate::vertexDataModified);
    }

    // Couple the buffer
    m_mappedVertexArray->SetNumberOfComponents(3);
    m_mappedVertexArray->SetArray(reinterpret_cast<double*>(m_vertices->getPointer()), m_vertices->size() * 3, 1);
    m_mappedVertexArray->Modified();
    m_polydata->GetPoints()->SetNumberOfPoints(m_vertices->size());
}

void
VTKSurfaceMeshRenderDelegate::setNormalBuffer(std::shared_ptr<VecDataArray<double, 3>> normals)
{
    // If the buffer changed
    if (m_normals != normals)
    {
        // If previous buffer exist
        if (m_normals != nullptr)
        {
            // stop observing its changes
            disconnect(m_normals, this, &VecDataArray<double, 3>::modified);
        }
        // Set new buffer and observe
        m_normals = normals;
        queueConnect<Event>(m_normals, &VecDataArray<double, 3>::modified, this, &VTKSurfaceMeshRenderDelegate::normalDataModified);
    }

    // Couple the buffer
    m_mappedNormalArray->SetNumberOfComponents(3);
    m_mappedNormalArray->SetArray(reinterpret_cast<double*>(m_normals->getPointer()), m_normals->size() * 3, 1);
    m_mappedNormalArray->Modified();
}

void
VTKSurfaceMeshRenderDelegate::setIndexBuffer(std::shared_ptr<VecDataArray<int, 3>> indices)
{
    // If the buffer changed
    if (m_indices != indices)
    {
        // If previous buffer exist
        if (m_indices != nullptr)
        {
            // stop observing its changes
            disconnect(m_indices, this, &VecDataArray<int, 3>::modified);
        }
        // Set new buffer and observe
        m_indices = indices;
        queueConnect<Event>(m_indices, &VecDataArray<int, 3>::modified, this, &VTKSurfaceMeshRenderDelegate::indexDataModified);
    }

    // Copy the buffer
    // Copy cells
    m_cellArray->Reset();
    vtkIdType cell[3];
    for (const auto& t : *m_indices)
    {
        for (size_t i = 0; i < 3; i++)
        {
            cell[i] = t[i];
        }
        m_cellArray->InsertNextCell(3, cell);
    }
    m_cellArray->Modified();
}

void
VTKSurfaceMeshRenderDelegate::setVertexScalarBuffer(std::shared_ptr<AbstractDataArray> scalars)
{
    // If the buffer changed
    if (m_vertexScalars != scalars)
    {
        // If previous buffer exist
        if (m_vertexScalars != nullptr)
        {
            // stop observing its changes
            disconnect(m_vertexScalars, this, &AbstractDataArray::modified);
        }
        // Set new buffer and observe
        m_vertexScalars = scalars;
        queueConnect<Event>(m_vertexScalars, &AbstractDataArray::modified, this, &VTKSurfaceMeshRenderDelegate::vertexScalarsModified);
        m_mappedVertexScalarArray = GeometryUtils::coupleVtkDataArray(m_vertexScalars);
        m_polydata->GetPointData()->SetScalars(m_mappedVertexScalarArray);
    }
    m_mappedVertexScalarArray->SetNumberOfComponents(m_vertexScalars->getNumberOfComponents());
    m_mappedVertexScalarArray->SetVoidArray(m_vertexScalars->getVoidPointer(),
        static_cast<vtkIdType>(m_vertexScalars->size()), 1);
    m_mappedVertexScalarArray->Modified();
}

void
VTKSurfaceMeshRenderDelegate::setCellScalarBuffer(std::shared_ptr<AbstractDataArray> scalars)
{
    // If the buffer changed
    if (m_cellScalars != scalars)
    {
        // If previous buffer exist
        if (m_cellScalars != nullptr)
        {
            // stop observing its changes
            disconnect(m_cellScalars, this, &AbstractDataArray::modified);
        }
        // Set new buffer and observe
        m_cellScalars = scalars;
        queueConnect<Event>(m_cellScalars, &AbstractDataArray::modified, this, &VTKSurfaceMeshRenderDelegate::cellScalarsModified);
        m_mappedCellScalarArray = GeometryUtils::coupleVtkDataArray(m_cellScalars);
        m_polydata->GetCellData()->SetScalars(m_mappedCellScalarArray);
    }
    m_mappedCellScalarArray->SetNumberOfComponents(m_cellScalars->getNumberOfComponents());
    m_mappedCellScalarArray->SetVoidArray(m_cellScalars->getVoidPointer(),
        static_cast<vtkIdType>(m_cellScalars->size()), 1);
    m_mappedCellScalarArray->Modified();
}

void
VTKSurfaceMeshRenderDelegate::setTextureCoordinateBuffer(std::shared_ptr<AbstractDataArray> textureCoordinates)
{
    // If the buffer changed
    if (m_textureCoordinates != textureCoordinates)
    {
        // If previous buffer exist
        if (m_textureCoordinates != nullptr)
        {
            // stop observing its changes
            disconnect(m_textureCoordinates, this, &AbstractDataArray::modified);
        }
        // Set new buffer and observe
        m_textureCoordinates = textureCoordinates;
        queueConnect<Event>(m_textureCoordinates, &AbstractDataArray::modified, this, &VTKSurfaceMeshRenderDelegate::textureCoordinatesModified);
        m_mappedTCoordsArray = vtkFloatArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(textureCoordinates));
        m_mappedTCoordsArray->SetName(m_geometry->getActiveVertexTCoords().c_str());
        m_polydata->GetPointData()->SetTCoords(m_mappedTCoordsArray);
    }

    m_mappedTCoordsArray->SetNumberOfComponents(m_textureCoordinates->getNumberOfComponents());
    m_mappedTCoordsArray->SetVoidArray(m_textureCoordinates->getVoidPointer(), static_cast<vtkIdType>(m_textureCoordinates->size()), 1);
    m_mappedTCoordsArray->Modified();

    // Map Tangents
    if (m_geometry->getVertexTangents() == nullptr)
    {
        m_geometry->computeVertexTangents();
    }
    // These need to be float for PBR
    m_mappedTangentArray = vtkFloatArray::SafeDownCast(GeometryUtils::coupleVtkDataArray(m_geometry->getVertexTangents()));
    m_polydata->GetPointData()->SetTangents(m_mappedTangentArray);
    m_mappedTangentArray->Modified();
}

void
VTKSurfaceMeshRenderDelegate::initializeTextures()
{
    auto material = m_visualModel->getRenderMaterial();
    if (material == nullptr)
    {
        return;
    }

    unsigned int currentUnit = 0;

    // Go through all of the textures
    vtkSmartPointer<vtkActor> actor = vtkActor::SafeDownCast(m_actor);
    actor->GetProperty()->RemoveAllTextures();
    for (int unit = 0; unit < (int)Texture::Type::None; unit++)
    {
        // Get imstk texture
        auto texture = material->getTexture((Texture::Type)unit);
        // If neither of these are provided, the texture is not filled out
        if (texture->getImageData() == nullptr && texture->getPath() == "")
        {
            continue;
        }

        // Get vtk texture
        std::shared_ptr<TextureManager<VTKTextureDelegate>> textureManager  = m_textureManager.lock();
        auto                                                textureDelegate = textureManager->getTextureDelegate(texture);

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
        vtkSmartPointer<vtkTexture> currentTexture = textureDelegate->getVtkTexture();
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
                break;
            }
            case Texture::Type::AmbientOcclusion:
            case Texture::Type::ORM:
            {
                actor->GetProperty()->SetORMTexture(currentTexture);
                break;
            }
            case Texture::Type::Anistropy:
            {
                actor->GetProperty()->SetAnisotropyTexture(currentTexture);
                break;
            }
            case Texture::Type::CoatNormal:
            {
                actor->GetProperty()->SetCoatNormalTexture(currentTexture);
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

        currentUnit++;
    }
}
} // namespace imstk