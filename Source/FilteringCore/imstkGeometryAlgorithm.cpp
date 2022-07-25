/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkGeometryAlgorithm.h"
#include "imstkGeometry.h"
#include "imstkLogger.h"

namespace imstk
{
void
GeometryAlgorithm::setInput(std::shared_ptr<Geometry> inputGeometry, size_t port)
{
    if (m_inputs.count(port) == 0)
    {
        LOG(WARNING) << "Tried to set input " << port << " on filter with " << m_NumInputPorts << " ports";
    }

    m_inputs[port] = inputGeometry;
}

void
GeometryAlgorithm::setOutput(std::shared_ptr<Geometry> outputGeometry, const size_t port)
{
    if (m_outputs.count(port) == 0)
    {
        LOG(WARNING) << "Tried to set output " << port << " on filter with " << m_NumOutputPorts << " ports";
    }
    m_outputs[port] = outputGeometry;
}

void
GeometryAlgorithm::setNumInputPorts(const size_t numPorts)
{
    this->m_NumInputPorts = numPorts;
    // Add entries in the map for it
    for (size_t i = 0; i < numPorts; i++)
    {
        if (m_inputs.count(i) == 0)
        {
            m_inputs[i] = nullptr;
        }
    }
}

void
GeometryAlgorithm::setNumOutputPorts(const size_t numPorts)
{
    this->m_NumOutputPorts = numPorts;
    // Add entries in the map for it
    for (size_t i = 0; i < numPorts; i++)
    {
        if (m_outputs.count(i) == 0)
        {
            m_outputs[i] = nullptr;
        }
    }
}

bool
GeometryAlgorithm::areInputsValid()
{
    // Check input types
    for (const auto& port : m_inputs)
    {
        const size_t portId = port.first;
        Geometry*    input  = port.second.get();
        auto         found  = m_requiredTypeChecks.find(portId);
        if (found != m_requiredTypeChecks.cend())
        {
            // Require Input: can't be null has to succeed type check
            if (port.second == nullptr)
            {
                LOG(WARNING) << "GeometryAlgorithm input " << portId << " missing!";
                return false;
            }
            else if (!found->second(input))
            {
                LOG(WARNING) << "GeometryAlgorithm received invalid geometry type \"" <<
                    m_inputs.at(portId)->getTypeName() << "\" in port " << portId;
                return false;
            }
            continue;
        }

        found = m_optionalTypeChecks.find(portId);
        if (found != m_optionalTypeChecks.cend())
        {
            // Require Input: may be null, if not has succeed type check
            if (input != nullptr && !found->second(input))
            {
                LOG(WARNING) << "GeometryAlgorithm received invalid geometry type \"" <<
                    m_inputs.at(portId)->getTypeName() << "\" in port " << portId;
                return false;
            }
        }
    }
    return true;
}
} // namespace imstk