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

#include "imstkDynamicObject.h"
#include "imstkGeometryMap.h"
#include "imstkAbstractDynamicalModel.h"
#include "imstkComputeGraph.h"
#include "imstkLogger.h"

namespace imstk
{
size_t
DynamicObject::getNumOfDOF() const
{
    if (!m_dynamicalModel)
    {
        LOG(WARNING) << "Cannot get the degree of freedom since the dynamical model is not initialized! returning 0";
        return 0;
    }

    return m_dynamicalModel->getNumDegreeOfFreedom();
}

void
DynamicObject::updateGeometries()
{
    m_dynamicalModel->updatePhysicsGeometry();

    if (m_physicsToCollidingGeomMap)
    {
        m_physicsToCollidingGeomMap->apply();
    }

    if (m_updateVisualFromPhysicsGeometry)
    {
        if (m_physicsToVisualGeomMap)
        {
            m_physicsToVisualGeomMap->apply();
        }
    }
    else
    {
        CollidingObject::updateGeometries();
    }
}

void
DynamicObject::updatePhysicsGeometry()
{
    m_dynamicalModel->updatePhysicsGeometry();

    if (m_physicsToCollidingGeomMap)
    {
        m_physicsToCollidingGeomMap->apply();
    }
}

bool
DynamicObject::initialize()
{
    if (CollidingObject::initialize())
    {
        if (m_physicsToCollidingGeomMap)
        {
            m_physicsToCollidingGeomMap->initialize();
        }

        if (m_physicsToVisualGeomMap)
        {
            m_physicsToVisualGeomMap->initialize();
        }

        return m_dynamicalModel->initialize();
    }
    else
    {
        return false;
    }
}

void
DynamicObject::initGraphEdges(std::shared_ptr<ComputeNode> source, std::shared_ptr<ComputeNode> sink)
{
    // Copy, sum, and connect the model graph to nest within this graph
    m_computeGraph->addEdge(source, getUpdateNode());
    m_dynamicalModel->initGraphEdges();
    m_computeGraph->nestGraph(m_dynamicalModel->getComputeGraph(), getUpdateNode(), getUpdateGeometryNode());
    m_computeGraph->addEdge(getUpdateGeometryNode(), sink);
}

void
DynamicObject::reset()
{
    m_dynamicalModel->resetToInitialState();
    this->updateGeometries();
};
}