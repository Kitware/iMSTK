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
    if (!m_visualModels.empty())
    {
        return m_visualModels[0]->getGeometry();
    }
    return nullptr;
}

void
SceneObject::setVisualGeometry(std::shared_ptr<Geometry> geometry)
{
    if (m_visualModels.empty())
    {
        m_visualModels.push_back(std::make_shared<VisualModel>(geometry));
    }
    else
    {
        m_visualModels[0]->setGeometry(geometry);
    }
}

std::shared_ptr<Geometry>
SceneObject::getMasterGeometry() const
{
    return this->getVisualGeometry();
}

std::shared_ptr<VisualModel>
SceneObject::getVisualModel(unsigned int index)
{
    return m_visualModels[index];
}

void
SceneObject::addVisualModel(std::shared_ptr<VisualModel> visualModel)
{
    m_visualModels.push_back(visualModel);
}

const std::vector<std::shared_ptr<VisualModel>>&
SceneObject::getVisualModels()
{
    return m_visualModels;
}

const size_t
SceneObject::getNumVisualModels()
{
    return m_visualModels.size();
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
} // imstk
