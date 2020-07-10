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

#include <memory>
#include <unordered_map>

#define imstkSetMacro(name, dataType)       \
    virtual void set ## name(dataType _arg) \
        {                                   \
                if (this->name != _arg)     \
        {                                   \
            this->name = _arg;              \
                }                           \
    }
#define imstkGetMacro(name, dataType) \
    virtual dataType get ## name() { return this->name; }

namespace imstk
{
class Geometry;

// Base class for geometry algorithms
class GeometryAlgorithm
{
protected:
    GeometryAlgorithm() = default;

public:
    GeometryAlgorithm(const GeometryAlgorithm&)  = delete;
    GeometryAlgorithm(const GeometryAlgorithm&&) = delete;
    virtual ~GeometryAlgorithm() = default;

public:
    ///
    /// \brief Returns input geometry given port, returns nullptr if doesn't exist
    ///
    std::shared_ptr<Geometry> getInput(size_t port = 0) const { return (m_inputs.count(port) == 0) ? nullptr : m_inputs.at(port); }
    ///
    /// \brief Returns output geometry given port, returns nullptr if doesn't exist
    ///
    std::shared_ptr<Geometry> getOutput(size_t port = 0) const { return m_outputs.count(port) == 0 ? nullptr : m_outputs.at(port); }

    ///
    /// \brief Set the input at the port
    ///
    void setInput(std::shared_ptr<Geometry> inputGeometry, size_t port = 0);

protected:
    ///
    /// \brief Set the output at the port
    ///
    void setOutput(std::shared_ptr<Geometry> inputGeometry, size_t port = 0);

    imstkGetMacro(NumberOfInputPorts, size_t);
    imstkGetMacro(NumberOfOutputPorts, size_t);

    ///
    /// \brief Sets the amount of input ports
    ///
    void setNumberOfInputPorts(size_t numPorts);

    ///
    /// \brief Sets the amount of output ports
    ///
    void setNumberOfOutputPorts(size_t numPorts);

public:
    //void modified() { this->m_modified = true; }

    void update()
    {
        //if (m_modified)
        //{
        requestUpdate();
        //}
        //m_modified = false;
    }

protected:
    virtual void requestUpdate() = 0;

private:
    std::unordered_map<size_t, std::shared_ptr<Geometry>> m_inputs;
    std::unordered_map<size_t, std::shared_ptr<Geometry>> m_outputs;
    //bool m_modified = true;
    size_t NumberOfInputPorts  = 1;
    size_t NumberOfOutputPorts = 1;
};
}