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

#include "imstkEventObject.h"
#include "imstkMacros.h"
#include "imstkTextureManager.h"
#include "imstkVTKTextureDelegate.h"

#include <vtkSmartPointer.h>

class vtkAbstractMapper;
class vtkProp3D;
class vtkTransform;
class vtkTexture;

namespace imstk
{
class Texture;
class RenderMaterial;
class VisualModel;

///
/// \class VTKRenderDelegate
///
/// \brief Base class for VTK render delegates
///
class VTKRenderDelegate : public EventObject
{
public:
    ~VTKRenderDelegate() override = default;

    ///
    /// \brief Instantiate proper render delegate
    ///
    static std::shared_ptr<VTKRenderDelegate> makeDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Return geometry to render
    ///
    std::shared_ptr<VisualModel> getVisualModel() const { return m_visualModel; }

    ///
    /// \brief Get VTK actor
    ///
    vtkSmartPointer<vtkProp3D> getVtkActor() { return m_actor; }

    ///
    /// \brief Set the TextureManager
    ///
    void setTextureManager(std::weak_ptr<TextureManager<VTKTextureDelegate>> textureManager) { m_textureManager = textureManager; }

    ///
    /// \brief Update render delegate
    ///
    void update();

    ///
    /// \brief Process the event queue, default implementation processes
    /// visualModel events and its RenderMaterial events
    ///
    virtual void processEvents();

    ///
    /// \brief Updates the actor and mapper properties from the currently set VisualModel
    ///
    virtual void updateRenderProperties() = 0;

protected:
    VTKRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Callback for updating render properties
    ///
    void materialModified(Event* imstkNotUsed(e)) { updateRenderProperties(); }

    ///
    /// \brief Callback for visual model modifications
    ///
    void visualModelModified(Event* e);

    vtkSmartPointer<vtkTexture> getVTKTexture(std::shared_ptr<Texture> texture);

    vtkSmartPointer<vtkTransform> m_transform;

    // VTK data members used to create the rendering pipeline
    vtkSmartPointer<vtkProp3D> m_actor;
    vtkSmartPointer<vtkAbstractMapper> m_mapper;

    std::shared_ptr<VisualModel>    m_visualModel; ///< imstk visual model (contains data (geometry) and render specification (render material))
    std::shared_ptr<RenderMaterial> m_material;

    std::weak_ptr<TextureManager<VTKTextureDelegate>> m_textureManager;
};
} // namespace imstk
