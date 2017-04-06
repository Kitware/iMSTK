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

#ifndef imstkVTKRenderDelegate_h
#define imstkVTKRenderDelegate_h

#include <memory>

#include "imstkGeometry.h"

#include "vtkSmartPointer.h"
#include "vtkAlgorithmOutput.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkTransform.h"

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
    /// \brief Default destructor
    ///
    ~VTKRenderDelegate() = default;

    ///
    /// \brief
    ///
    static std::shared_ptr<VTKRenderDelegate> make_delegate(std::shared_ptr<Geometry>geom);

    ///
    /// \brief
    ///
    void setUpMapper(vtkAlgorithmOutput *source, const bool rigid);

    ///
    /// \brief
    ///
    virtual std::shared_ptr<Geometry> getGeometry() const = 0;

    ///
    /// \brief
    ///
    vtkSmartPointer<vtkActor> getVtkActor() const;

    ///
    /// \brief
    ///
    virtual void update();

    ///
    /// \brief
    ///
    void updateActorTransform();

protected:
    ///
    /// \brief Default constructor (protected)
    ///
    VTKRenderDelegate()
    {
        m_actor = vtkSmartPointer<vtkActor>::New();
        m_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_transform = vtkSmartPointer<vtkTransform>::New();
        m_actor->SetMapper(m_mapper);
        m_actor->SetUserTransform(m_transform);
    }

    vtkSmartPointer<vtkActor> m_actor;
    vtkSmartPointer<vtkPolyDataMapper> m_mapper;
    vtkSmartPointer<vtkTransform> m_transform;
};
}

#endif // ifndef imstkRenderDelegate_h
