// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef CORE_CORECLASS_H
#define CORE_CORECLASS_H

// STL includes
#include <memory>
#include <atomic>
#include <iostream>
#include <map>

// iMSTK includes
#include "Core/Config.h"
#include "Core/UnifiedId.h"
#include "Core/EventHandler.h"
#include "Core/ConfigRendering.h"
#include "Core/RenderDetail.h"

namespace imstk {

class CoreClass;
class RenderDelegate;
class ObjectSimulator;

/// \brief simulator calls object and sends this structure
struct SimulationParam
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

    static std::shared_ptr<EventHandler> eventHandler;

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
    /// \brief Default constructor
    ///
    virtual ~CoreClass() = default;

    ///
    /// \brief get type of the class
    ///
    const ClassType &getType() const ;

    ///
    /// \brief set type of the class
    ///
    void setType(const ClassType &newType);

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
    virtual void initSimulate(const SimulationParam &p_params);

    ///
    /// \brief simulates the object
    ///
    virtual void simulate(const SimulationParam &p_params);

    ///
    /// \brief print the object
    ///
    virtual void print() const;

    ///
    /// \brief event binding function
    /// This function is called by the event handler after observing
    /// events.
    ///
    virtual void handleEvent(std::shared_ptr<Event>);

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
    const EventHandler::FunctionContainerType::iterator
    &getEventIndex(const EventType &eventType) const
    { return eventIndexMap.at(eventType); }

    ///
    /// \brief Set event index used by the event handler to unregister event observers
    ///
    void setEventIndex(const EventType &eventType, EventHandler::FunctionContainerType::iterator index)
    { eventIndexMap[eventType] = index; }

    ///
    /// \brief Set event index used by the event handler to unregister event observers
    ///
    void removeEventIndex(const EventType &eventType)
    { eventIndexMap.erase(eventType); }

    ///
    /// \brief Set the order on which the objects are painted.
    ///
    void setDrawOrder(const ClassDrawOrder &order) { drawOrder = order; }

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

    void attachEvent(const EventType &eventType, std::shared_ptr<CoreClass> component)
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
    ClassType type; ///< class type
    std::string name; ///< name of the class
    bool listening; ///< parameter to determine if this object is listening for events
    std::map<
    EventType,
    EventHandler::FunctionContainerType::iterator> eventIndexMap;
    std::shared_ptr<RenderDelegate> renderDelegate; ///!< Class that can render this class
    std::shared_ptr<RenderDetail> renderDetail; ///< specifies visualization type

private:
    std::shared_ptr<UnifiedId> uniqueId; ///< unique Id
    ClassDrawOrder drawOrder; ///< draw order of the object
};

}

#endif
