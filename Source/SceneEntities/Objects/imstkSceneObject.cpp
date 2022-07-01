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
#include "imstkGeometry.h"
#include "imstkTaskGraph.h"
#include "imstkVisualModel.h"

#include "imstkLogger.h"

namespace imstk
{
SceneObject::SceneObject(const std::string& name) : SceneEntity(), m_name(name),
    m_taskGraph(std::make_shared<TaskGraph>(
        "SceneObject_" + m_name + "_Source",
        "SceneObject_" + m_name + "_Sink"))
{
    m_updateNode = m_taskGraph->addFunction("SceneObject_" + m_name + "_Update", [this](){ update(); });
    m_updateGeometryNode = m_taskGraph->addFunction("SceneObject_" + m_name + "_UpdateGeometry", [this]() { updateGeometries(); });
}

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
        auto visualModel = std::make_shared<VisualModel>();
        visualModel->setGeometry(geometry);
        m_visualModels.push_back(visualModel);
    }
    else
    {
        m_visualModels[0]->setGeometry(geometry);
    }
}

void
SceneObject::initGraphEdges()
{
    m_taskGraph->clearEdges();
    initGraphEdges(m_taskGraph->getSource(), m_taskGraph->getSink());
}

void
SceneObject::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    m_taskGraph->addEdge(source, m_updateNode);
    m_taskGraph->addEdge(m_updateNode, m_updateGeometryNode);
    m_taskGraph->addEdge(m_updateGeometryNode, sink);
}
} // namespace imstk