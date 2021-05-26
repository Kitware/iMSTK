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

///
/// \class GeometryAlgorithm
///
/// \brief Base abstract class for geometry algorithms. GeometryAlgorithms take N input
/// geometries and produce N output geometries. Sublcasses should implement requestUpdate
/// to do algorithm logic. Subclasses may also setInputPortReq to require an input to be
/// a certain type.
///
class GeometryAlgorithm
{
public:
    ///
    /// \brief Used for type erasure of the port requirements
    /// \todo: Type names can't be deduced for abstract classes.
    /// Would be nice to have static type name as well
    ///
    class PortReq
    {
    public:
        struct BaseReq
        {
            virtual ~BaseReq() = default;
            virtual bool isValid(std::shared_ptr<Geometry> geom) const = 0;
            //virtual std::string name() const = 0;
        };

        template<typename T>
        struct Requirement : public BaseReq
        {
            virtual ~Requirement() override = default;
            bool isValid(std::shared_ptr<Geometry> geom) const override
            {
                return std::dynamic_pointer_cast<T>(geom) != nullptr;
            }

            //std::string name() const override
            //{
            //    T t; // Can't use
            //    return t.getTypeName();
            //}
        };

        std::shared_ptr<BaseReq> req = nullptr;

    public:
        PortReq() = default;

        template<typename T>
        PortReq(T*) : req(std::make_shared<Requirement<T>>()) { }

        bool isValid(std::shared_ptr<Geometry> geom) const { return req->isValid(geom); }
        //std::string validGeomName() const { return req->name(); }
    };

protected:
    GeometryAlgorithm() = default;

public:
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

    ///
    /// \brief Set a type requirement on the inputs, it will check when running
    /// the algorithm and warn at runtime
    ///
    template<typename T>
    void setInputPortReq(size_t port)
    {
        T* ptr = nullptr;
        m_inputPortTypeReqs[port] = PortReq(ptr);
    }

public:
    //void modified() { this->m_modified = true; }

    ///
    /// \brief Do the actual algorithm
    ///
    void update()
    {
        if (!checkInputRequirements(m_inputs, m_inputPortTypeReqs))
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
    /// \brief Check inputs are correct
    /// \return true if all inputs match the requirements, false if not
    ///
    virtual bool checkInputRequirements(
        const std::unordered_map<size_t, std::shared_ptr<Geometry>>& inputs,
        const std::unordered_map<size_t, PortReq>& inputPortReqs);

    ///
    /// \brief Users can implement this for the logic to be run
    ///
    virtual void requestUpdate() = 0;

private:
    std::unordered_map<size_t, PortReq> m_inputPortTypeReqs; // The desired types of the input
    std::unordered_map<size_t, std::shared_ptr<Geometry>> m_inputs;
    std::unordered_map<size_t, std::shared_ptr<Geometry>> m_outputs;
    //bool m_modified = true;
    size_t m_NumberOfInputPorts  = 1;
    size_t m_NumberOfOutputPorts = 1;
};
}