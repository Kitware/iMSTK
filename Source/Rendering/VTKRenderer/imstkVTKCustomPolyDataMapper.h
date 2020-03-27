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

#include <fstream>
#include <sstream>

#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLBufferObject.h"
#include "vtkObjectFactory.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkOpenGLVertexArrayObject.h"
#include "vtkOpenGLVertexBufferObject.h"
#include "vtkOpenGLVertexBufferObjectCache.h"
#include "vtkOpenGLVertexBufferObjectGroup.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkRenderWindow.h"
#include "vtkShaderProgram.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkMatrix4x4.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLActor.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkLightCollection.h"
#include "vtkLight.h"
#include "vtkOpenGLTexture.h"
#include "vtkOpenGLError.h"

#include "imstkGeometry.h"
#include "imstkSurfaceMesh.h"
#include "imstkRenderMaterial.h"

namespace imstk
{
///
/// \class VTKCustomPolyDataMapper
///
/// \brief Custom interface between shaders and iMSTK.
///
/// This class overrides behavior in VTK regarding shading.
///
class VTKCustomPolyDataMapper : public vtkOpenGLPolyDataMapper
{
public:
    vtkTypeMacro(VTKCustomPolyDataMapper, vtkOpenGLPolyDataMapper);

    static VTKCustomPolyDataMapper* New();

    ///
    /// \brief Set the render material
    ///
    void setRenderMaterial(std::shared_ptr<RenderMaterial> renderMat);

    ///
    /// \brief Let the polydata mapper know if it is for the surface mesh
    ///
    void setIsSurfaceMapper(const bool val) { m_isSurfaceMapper = val; };

protected:
    ///
    /// \brief Sets up the VBO and VAO
    ///
    void BuildBufferObjects(vtkRenderer* renderer, vtkActor* actor) override;

    ///
    /// \brief Overridden method to prevent shader overwriting
    ///
    virtual void ReplaceShaderValues(
        std::map<vtkShader::Type, vtkShader*> shaders,
        vtkRenderer* renderer,
        vtkActor* actor) override;

    ///
    /// \brief Loads the shader and injects preprocessor commands
    ///
    virtual void GetShaderTemplate(
        std::map<vtkShader::Type, vtkShader*> shaders,
        vtkRenderer* renderer,
        vtkActor* actor) override;

    ///
    /// \brief Does all of the uniform/texture setting
    ///
    virtual void SetMapperShaderParameters(
        vtkOpenGLHelper& helper,
        vtkRenderer*     renderer,
        vtkActor*        actor) override;

    ///
    /// \brief Overwritten to prevent extra uniform assignment
    ///
    virtual void SetCameraShaderParameters(
        vtkOpenGLHelper& helper,
        vtkRenderer*     renderer,
        vtkActor*        actor) override;

    ///
    /// \brief Overwritten to prevent extra uniform assignment
    ///
    virtual void SetLightingShaderParameters(
        vtkOpenGLHelper& helper,
        vtkRenderer*     renderer,
        vtkActor*        actor) override;

    ///
    /// \brief Overwritten to prevent extra uniform assignment
    ///
    virtual void SetPropertyShaderParameters(
        vtkOpenGLHelper& helper,
        vtkRenderer*     renderer,
        vtkActor*        actor) override;

    ///
    /// \brief Allows for debugging interaction with VTK
    ///
    virtual void UpdateShaders(vtkOpenGLHelper& helper,
                               vtkRenderer*     renderer,
                               vtkActor*        actor) override;

    ///
    /// \brief Loads a shader
    ///
    void loadShader(const std::string& filename, std::string& source);

    std::shared_ptr<RenderMaterial> m_renderMaterial; ///< Geometry reference

    std::string m_vertexShaderSource   = "";          ///< Source for vertex shader
    std::string m_fragmentShaderSource = "";          ///< Source for fragment shader

    vtkOpenGLBufferObject* m_positionsVBO;            ///< Vertex positions VBO
    vtkOpenGLBufferObject* m_normalsVBO;              ///< Vertex normals VBO
    vtkOpenGLBufferObject* m_uvVBO;                   ///< Vertex UVs VBO

    bool m_isSurfaceMapper = false;
};
}
