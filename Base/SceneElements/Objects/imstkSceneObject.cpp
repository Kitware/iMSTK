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

#include "imstkSceneObject.h"

#include "imstkSceneObjectController.h"
#include "imstkDeviceClient.h"

namespace imstk
{

std::shared_ptr<Geometry>
SceneObject::getVisualGeometry() const
{
    return m_visualGeometry;
}

void
SceneObject::setVisualGeometry(std::shared_ptr<Geometry> geometry)
{
    m_visualGeometry = geometry;
}

std::shared_ptr<Geometry>
SceneObject::getMasterGeometry() const
{
    return m_visualGeometry;
}

const SceneObject::Type&
SceneObject::getType() const
{
    return m_type;
}

void
SceneObject::setType(SceneObject::Type type)
{
    m_type = type;
}

const std::string&
SceneObject::getName() const
{
    return m_name;
}

void
SceneObject::setName(const std::string& name)
{
    m_name = name;
}

std::shared_ptr<SceneObjectController>
SceneObject::getController() const
{
    return m_controller;
}

std::shared_ptr<SceneObjectController>
SceneObject::setupController(std::shared_ptr<DeviceClient> deviceClient)
{
    if(m_controller == nullptr)
    {
        m_controller = std::make_shared<SceneObjectController>(*this, deviceClient);
    }
    else
    {
        m_controller->setDeviceClient(deviceClient);
    }
    return m_controller;
}

} // imstk
