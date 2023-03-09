/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAbstractDynamicalSystem.h"
#include "imstkLogger.h"
#include "imstkTaskGraph.h"

namespace imstk
{
AbstractDynamicalSystem::AbstractDynamicalSystem(DynamicalModelType type) :
    m_type(type), m_numDof(0), m_taskGraph(std::make_shared<TaskGraph>("AbstractDynamicalModel_Source", "AbstractDynamicalModel_Sink"))
{
}

bool
AbstractDynamicalSystem::isGeometryValid(const std::shared_ptr<Geometry> geometry)
{
    if (geometry)
    {
        // If no valid geometries specified all geometries work
        if (m_validGeometryTypes.size() == 0)
        {
            return true;
        }

        // If it exists in the set then it is valid geometry
        if (m_validGeometryTypes.count(geometry->getTypeName()))
        {
            return true;
        }
        else
        {
            LOG(WARNING) << "The geometry is not supported!!";
        }
    }
    else
    {
        LOG(WARNING) << "The geometry is not a valid pointer";
    }

    return false;
}

void
AbstractDynamicalSystem::setModelGeometry(std::shared_ptr<Geometry> geometry)
{
    if (isGeometryValid(geometry))
    {
        m_geometry = geometry;
    }
    else
    {
        LOG(WARNING) << "Invalid geometry for Model";
    }
}

void
AbstractDynamicalSystem::initGraphEdges()
{
    m_taskGraph->clearEdges();
    initGraphEdges(m_taskGraph->getSource(), m_taskGraph->getSink());
}

void
AbstractDynamicalSystem::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    m_taskGraph->addEdge(source, sink);
}
} // namespace imstk