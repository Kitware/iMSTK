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
#include "imstkComputeGraph.h"
#include "imstkComputeNode.h"

namespace imstk
{
SceneObject::SceneObject(const std::string& name) :
    m_type(Type::Visual), m_name(name),
    m_computeGraph(std::make_shared<ComputeGraph>("SceneObject_" + name + "_Source", "SceneObject_" + name + "_Sink")), SceneEntity()
{
    m_updateNode = m_computeGraph->addFunction("SceneObject_" + name + "_Update", [&]() { this->update(); });
    m_updateGeometryNode = m_computeGraph->addFunction("SceneObject_" + name + "_UpdateGeometry", [&]() { this->updateGeometries(); });
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
        m_visualModels.push_back(std::make_shared<VisualModel>(geometry));
    }
    else
    {
        m_visualModels[0]->setGeometry(geometry);
    }
}

void
SceneObject::initGraphEdges()
{
    m_computeGraph->clearEdges();
    initGraphEdges(m_computeGraph->getSource(), m_computeGraph->getSink());
}

void
SceneObject::initGraphEdges(std::shared_ptr<ComputeNode> source, std::shared_ptr<ComputeNode> sink)
{
    m_computeGraph->addEdge(source, m_updateNode);
    m_computeGraph->addEdge(m_updateNode, m_updateGeometryNode);
    m_computeGraph->addEdge(m_updateGeometryNode, sink);
}
} // imstk
