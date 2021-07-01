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
        LOG(WARNING) << "Tried to set input " << port << " on filter with " << m_NumberOfInputPorts << " ports";
    }

    m_inputs[port] = inputGeometry;
}

void
GeometryAlgorithm::setOutput(std::shared_ptr<Geometry> outputGeometry, size_t port)
{
    if (m_outputs.count(port) == 0)
    {
        LOG(WARNING) << "Tried to set output " << port << " on filter with " << m_NumberOfOutputPorts << " ports";
    }
    m_outputs[port] = outputGeometry;
}

void
GeometryAlgorithm::setNumberOfInputPorts(size_t numPorts)
{
    this->m_NumberOfInputPorts = numPorts;
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
GeometryAlgorithm::setNumberOfOutputPorts(size_t numPorts)
{
    this->m_NumberOfOutputPorts = numPorts;
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
GeometryAlgorithm::areInputsValid() const
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
}