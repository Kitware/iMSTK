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

#include "imstkLogger.h"
#include "imstkMacros.h"

#include <memory>
#include <unordered_map>

namespace imstk
{
class Geometry;

/// Returns a function that for instances with common Base classes determines
/// whether it is of the Target type
template<class Base, class Target>
std::function<bool(Base*)>
makeTypeCheck()
{
    return [](Base* p) {
               return (dynamic_cast<Target*>(p) != nullptr);
           };
}

///
/// \class GeometryAlgorithm
///
/// \brief Abstract base class for geometry algorithms. GeometryAlgorithms take N input
/// geometries and produce N output geometries. Subclasses should implement requestUpdate
/// to do algorithm logic. Subclasses may also setInputPortReq to require an input to be
/// a certain type.
///
class GeometryAlgorithm
{
protected:
    GeometryAlgorithm() = default;

public:
    virtual ~GeometryAlgorithm() = default;

    ///
    /// \brief Returns input geometry given port, returns nullptr if doesn't exist
    ///
    std::shared_ptr<Geometry> getInput(size_t port = 0) const
    {
        return (m_inputs.count(port) == 0) ? nullptr : m_inputs.at(port);
    }

    ///
    /// \brief Returns output geometry given port, returns nullptr if doesn't exist
    ///
    std::shared_ptr<Geometry> getOutput(size_t port = 0) const
    {
        return m_outputs.count(port) == 0 ? nullptr : m_outputs.at(port);
    }

    ///
    /// \brief Set the input at the port
    ///
    void setInput(std::shared_ptr<Geometry> inputGeometry, size_t port = 0);

    ///
    /// \brief Do the actual algorithm
    ///
    void update()
    {
        if (!areInputsValid())
        {
            LOG(WARNING) << "GeometryAlgorithm failed to run, inputs not satisfied";
            return;
        }
        //if (m_modified)
        //{
        requestUpdate();
        //}
        //m_modified = false;
    }

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

    ///
    /// \brief Declares the type for the port with the given number, also defines that
    /// the give port is required for the filter to run correctly
    ///
    template<typename T>
    void setRequiredInputType(size_t port)
    {
        CHECK(m_optionalTypeChecks.find(port) == m_optionalTypeChecks.end())
            << "There is already an optional type for this port " << port << ", can't assign another one.";
        m_requiredTypeChecks[port] = makeTypeCheck<Geometry, T>();
    }

    ///
    /// \brief Declares the type for the port with the given number, the data
    /// for this port is optional and may be omitted
    ///
    template<typename T>
    void setOptionalInputType(size_t port)
    {
        CHECK(m_requiredTypeChecks.find(port) == m_requiredTypeChecks.end())
            << "There is already a required type for port " << port << " , can't assign another one.";
        m_optionalTypeChecks[port] = makeTypeCheck<Geometry, T>();
    }

    ///
    /// \brief Users can implement this for the logic to be run
    ///
    virtual void requestUpdate() = 0;

    ///
    /// \brief Check inputs are correct
    /// \return true if all inputs match the requirements, false if not
    ///
    virtual bool areInputsValid();

    using GeometryCheck      = std::function<bool (Geometry*)>;
    using TypeCheckContainer = std::unordered_map<size_t, GeometryCheck>;

    std::unordered_map<size_t, GeometryCheck> m_requiredTypeChecks;
    std::unordered_map<size_t, GeometryCheck> m_optionalTypeChecks;

private:

    std::unordered_map<size_t, std::shared_ptr<Geometry>> m_inputs;
    std::unordered_map<size_t, std::shared_ptr<Geometry>> m_outputs;

    //bool m_modified = true;
    size_t m_NumberOfInputPorts  = 1;
    size_t m_NumberOfOutputPorts = 1;
};
}