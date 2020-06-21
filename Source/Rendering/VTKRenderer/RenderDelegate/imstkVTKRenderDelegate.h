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

#include "imstkGeometry.h"
#include "imstkRenderMaterial.h"
#include "imstkVTKTextureDelegate.h"
#include "imstkTextureManager.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkVisualModel.h"

#include <vtkSmartPointer.h>
#include <vtkAlgorithmOutput.h>
#include <vtkActor.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkProperty.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>

namespace imstk
{
///
/// \class VTKRenderDelegate
///
/// \brief Base class for VTK render delegates
///
class VTKRenderDelegate
{
public:

    ///
    /// \brief Instantiate proper render delegate
    ///
    static std::shared_ptr<VTKRenderDelegate> makeDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Instantiate proper debug render delegate
    ///
    static std::shared_ptr<VTKRenderDelegate> makeDebugDelegate(std::shared_ptr<VisualModel> dbgVizModel);

    ///
    /// \brief Set up normals and mapper
    /// \param source input data object
    /// \param vizModel imstk visual model
    ///
    void setUpMapper(vtkAlgorithmOutput* source, const std::shared_ptr<VisualModel> vizModel);

    ///
    /// \brief Return geometry to render
    ///
    std::shared_ptr<VisualModel> getVisualModel() const;

    ///
    /// \brief Get VTK renderered object
    ///
    vtkProp3D* getVtkActor() const;

    ///
    /// \brief Update render delegate
    ///
    void update();

    ///
    /// \brief Update renderer delegate transform based on the geometry shallow transform
    ///
    void updateActorTransform();

    ///
    /// \brief Update render delegate properties based on the geometry render material
    ///
    void updateActorProperties();
    void updateActorPropertiesVolumeRendering();
    void updateActorPropertiesMesh();
    bool isMesh() { return m_isMesh; }
    bool isVolume() { return m_modelIsVolume; }

    ///
    /// \brief Update render delegate source based on the geometry internal data
    ///
    virtual void updateDataSource() = 0;

protected:

    vtkSmartPointer<vtkTexture> getVTKTexture(std::shared_ptr<Texture> texture);

    ///
    /// \brief Default constructor (protected)
    ///
    VTKRenderDelegate() :
        m_actor(vtkSmartPointer<vtkActor>::New()),
        m_mapper(vtkSmartPointer<vtkOpenGLPolyDataMapper>::New()),
        m_transform(vtkSmartPointer<vtkTransform>::New()),
        m_volumeMapper(vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New()),
        m_volume(vtkSmartPointer<vtkVolume>::New()),
        m_modelIsVolume(false)               // remove?
    {
        m_actor->SetMapper(m_mapper);        // remove this as a default since it could be volume mapper?
        m_actor->SetUserTransform(m_transform);
        m_volume->SetMapper(m_volumeMapper); // remove this as a default?
    }

    virtual ~VTKRenderDelegate() = default;

    vtkSmartPointer<vtkTransform> m_transform;

    // volume mapping
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_volumeMapper;
    vtkSmartPointer<vtkVolume> m_volume;
    bool m_modelIsVolume = false;// remove?

    bool m_isMesh = true;

    // VTK data members used to create the rendering pipeline
    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkOpenGLPolyDataMapper> m_mapper;

    std::shared_ptr<VisualModel> m_visualModel; ///< imstk visual model (contains data (geometry) and render specification (render material))
};
}
