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
#include "imstkVTKCustomPolyDataMapper.h"
#include "imstkDebugRenderGeometry.h"
#include "imstkVisualModel.h"

#include <vtkSmartPointer.h>
#include <vtkAlgorithmOutput.h>
#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkProperty.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>

namespace imstk
{
///
/// \class RenderDelegate
///
/// \brief Base class for render delegates
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
    /// \param notSurfaceMesh if the mesh isn't a surface Mesh
    /// \param geometry the geometry object
    ///
    void setUpMapper(vtkAlgorithmOutput*             source,
                     const bool                      notSurfaceMesh,
                     std::shared_ptr<RenderMaterial> renderMat);

    ///
    /// \brief Return geometry to render
    ///
    std::shared_ptr<VisualModel> getVisualModel() const;

    ///
    /// \brief Get VTK renderered object
    ///
    vtkSmartPointer<vtkProp3D> getVtkActor() const;

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

    ///
    /// \brief Update render delegate source based on the geometry internal data
    ///
    virtual void updateDataSource() = 0;

protected:
    ///
    /// \brief Default constructor (protected)
    ///
    VTKRenderDelegate() :
        m_actor(vtkSmartPointer<vtkActor>::New()),
        m_mapper(vtkSmartPointer<VTKCustomPolyDataMapper>::New()),
        m_transform(vtkSmartPointer<vtkTransform>::New()),
        m_volumeMapper(vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New()),
        m_volume(vtkSmartPointer<vtkVolume>::New()),
        m_modelIsVolume(false)
    {
        m_actor->SetMapper(m_mapper);
        m_actor->SetUserTransform(m_transform);
        m_volume->SetMapper(m_volumeMapper);
    }

    virtual ~VTKRenderDelegate() = default;

    std::shared_ptr<VisualModel> m_visualModel;
    vtkSmartPointer<vtkActor>    m_actor;
    vtkSmartPointer<VTKCustomPolyDataMapper> m_mapper;
    vtkSmartPointer<vtkTransform> m_transform;
    vtkSmartPointer<vtkGPUVolumeRayCastMapper> m_volumeMapper;
    vtkSmartPointer<vtkVolume> m_volume;
    bool m_modelIsVolume;
};
}
