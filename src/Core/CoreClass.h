// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMCORECLASS_H
#define SMCORECLASS_H

// STL includes
#include <memory>
#include <atomic>
#include <iostream>
#include <map>

// SimMedTK includes
#include "Config.h"
#include "UnifiedId.h"
#include "EventHandler.h"
#include "Rendering/ConfigRendering.h"

class SDK;
class CoreClass;
class RenderDelegate;
class ObjectSimulator;
class Viewer;

/// \brief simulator calls object and sends this structure
struct smSimulationParam
{
    std::shared_ptr<ObjectSimulator> objectSimulator;
    void *caller;
    void *data;
};

///
/// \brief new operator
///
/// This class should be used as :
///    MyClass::Pointer p = New<MyT>(args);
///
///
template<typename T, typename Pointer = std::shared_ptr<T>>
class New : public Pointer
{
public:
    template<typename... ArgsType>
    New(ArgsType&&... args)
    {
        Pointer(std::make_shared<T>(args...));
    }
};


/// \brief core base class of all objects in framework
class CoreClass : public std::enable_shared_from_this<CoreClass>
{
public:
    using Pointer = std::shared_ptr<CoreClass>;

    static std::shared_ptr<core::EventHandler> eventHandler;

public:
    ///
    /// \brief Default constructor
    ///
    CoreClass();

    ///
    /// \brief Default constructor
    ///
    CoreClass(const std::string &);

    ///
    /// \brief get type of the class
    ///
    const core::ClassType &getType() const ;

    ///
    /// \brief set type of the class
    ///
    void setType(const core::ClassType &newType);

    ///
    /// \brief his function is called by the renderer.
    ///
    virtual void initDraw();

    ///
    /// \brief draw function is called for visualization the object
    ///
    virtual void draw() const;

    ///
    /// \brief initialization of simulation
    ///
    virtual void initSimulate(const smSimulationParam &p_params);

    ///
    /// \brief simulates the object
    ///
    virtual void simulate(const smSimulationParam &p_params);

    ///
    /// \brief print the object
    ///
    virtual void print() const;

    ///
    /// \brief event binding function
    /// This function is called by the event handler after observing
    /// events.
    ///
    virtual void handleEvent(std::shared_ptr<core::Event>);

    ///
    /// \brief set the name of object
    ///
    void setName(const std::string &p_objectName);

    ///
    /// \brief get the name of the object
    ///
    const std::string &getName() const;

    ///
    /// \brief Increease reference counter
    ///
    std::atomic_int &operator++()
    {
        ++referenceCounter;
        return referenceCounter;
    }

    ///
    /// \brief Decreease reference counter
    ///
    std::atomic_int &operator--()
    {
        --referenceCounter;
        return referenceCounter;
    }

    ///
    /// \brief Event listening state
    /// \return listening
    ///
    const bool &isListening() const { return listening; }

    ///
    /// \brief Event listening state
    ///
    bool &isListening() { return listening; }

    ///
    /// \brief Event listening state
    ///
    void setIsListening( bool islistening) { listening = islistening; }

    ///
    /// \brief Event index used by the event handler to unregister event observers
    /// \return eventIndex
    ///
    const core::EventHandler::FunctionContainerType::iterator
    &getEventIndex(const core::EventType &eventType) const
    { return eventIndexMap.at(eventType); }

    ///
    /// \brief Set event index used by the event handler to unregister event observers
    ///
    void setEventIndex(const core::EventType &eventType, core::EventHandler::FunctionContainerType::iterator index)
    { eventIndexMap[eventType] = index; }

    ///
    /// \brief Set event index used by the event handler to unregister event observers
    ///
    void removeEventIndex(const core::EventType &eventType)
    { eventIndexMap.erase(eventType); }

    ///
    /// \brief Set the order on which the objects are painted.
    ///
    void setDrawOrder(const core::ClassDrawOrder &order) { drawOrder = order; }

    ///
    /// \brief Get the unique id of this object
    ///
    std::shared_ptr<UnifiedId> getUniqueId() {return uniqueId;}

    /// \brief Get render detail
    std::shared_ptr<RenderDetail> getRenderDetail() const
    {return renderDetail;}

    /// \brief Set the render details (properties affecting visual depiction)
    void setRenderDetail(std::shared_ptr<RenderDetail> newRenderDetail)
    { this->renderDetail = newRenderDetail; }

    std::shared_ptr<RenderDelegate> getRenderDelegate() const;
    void setRenderDelegate(std::shared_ptr<RenderDelegate> delegate);

    void attachEvent(const core::EventType &eventType, std::shared_ptr<CoreClass> component)
    {
        eventHandler->attachEvent(eventType,component);
    }

protected:
    ///
    /// \brief Allows to use the *this* pointer from any child
    ///
    template<typename DerivedType>
    inline std::shared_ptr<DerivedType> safeDownCast()
    {
        return std::static_pointer_cast<DerivedType>(shared_from_this());
    }

protected:
    std::atomic_int referenceCounter; ///< reference counter to identify the count the usage
    core::ClassType type; ///< class type
    std::string name; ///< name of the class
    bool listening; ///< parameter to determine if this object is listening for events
    std::map<
    core::EventType,
    core::EventHandler::FunctionContainerType::iterator> eventIndexMap;
    std::shared_ptr<RenderDelegate> renderDelegate; ///!< Class that can render this class

private:
    std::shared_ptr<UnifiedId> uniqueId; ///< unique Id
    std::shared_ptr<RenderDetail> renderDetail; ///< specifies visualization type
    core::ClassDrawOrder drawOrder; ///< draw order of the object
};

#endif
