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
#include "smCore/smConfig.h"
#include "smCore/smUnifiedId.h"
#include "smRendering/smConfigRendering.h"
#include "smEvent/smEventHandler.h"

class smSDK;
class smCoreClass;
class smObjectSimulator;
class smViewer;

/// \brief  viewer sends this to all objects to be rendered
struct smDrawParam
{
    smDrawParam();

    std::shared_ptr<smViewer> rendererObject;
    std::shared_ptr<smCoreClass> caller;
    void *data;
};

/// \brief simulator calls object and sends this structure
struct smSimulationParam
{
    std::shared_ptr<smObjectSimulator> objectSimulator;
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
class smCoreClass : public std::enable_shared_from_this<smCoreClass>
{
public:
    using Pointer = std::shared_ptr<smCoreClass>;

    static std::shared_ptr<smtk::Event::smEventHandler> eventHandler;

public:
    ///
    /// \brief Default constructor
    ///
    smCoreClass();

    ///
    /// \brief Default constructor
    ///
    smCoreClass(const std::string &);

    ///
    /// \brief get type of the class
    ///
    const smClassType &getType() const ;

    ///
    /// \brief set type of the class
    ///
    void setType(const smClassType &newType);

    ///
    /// \brief his function is called by the renderer. The p_params stores renderer pointers
    ///
    virtual void initDraw(const smDrawParam &p_params);

    ///
    /// \brief draw function is called for visualization the object
    ///
    virtual void draw(const smDrawParam &p_params);

    ///
    /// \brief draw function is called for visualization the object
    ///
    virtual void draw();

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
    virtual void handleEvent(std::shared_ptr<smtk::Event::smEvent>);

    ///
    /// \brief set the name of object
    ///
    void setName(const smString &p_objectName);

    ///
    /// \brief get the name of the object
    ///
    const smString &getName() const;

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
    const smtk::Event::smEventHandler::FunctionContainerType::iterator
    &getEventIndex(const smtk::Event::EventType &eventType) const
    { return eventIndexMap.at(eventType); }

    ///
    /// \brief Set event index used by the event handler to unregister event observers
    ///
    void setEventIndex(const smtk::Event::EventType &eventType, smtk::Event::smEventHandler::FunctionContainerType::iterator index)
    { eventIndexMap[eventType] = index; }

    ///
    /// \brief Set event index used by the event handler to unregister event observers
    ///
    void removeEventIndex(const smtk::Event::EventType &eventType)
    { eventIndexMap.erase(eventType); }

    ///
    /// \brief Set the order on which the objects are painted.
    ///
    void setDrawOrder(const smClassDrawOrder &order) { drawOrder = order; }

    ///
    /// \brief Get the unique id of this object
    ///
    std::shared_ptr<smUnifiedId> getUniqueId() {return uniqueId;}

    ///
    /// \brief Set the unique id of this object
    ///
    std::shared_ptr<smRenderDetail> getRenderDetail() {return renderDetail;}

    void attachEvent(const smtk::Event::EventType &eventType, std::shared_ptr<smCoreClass> component)
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
    smClassType type; ///< class type
    smString name; ///< name of the class
    bool listening; ///< parameter to determine if this object is listening for events
    std::map<
    smtk::Event::EventType,
    smtk::Event::smEventHandler::FunctionContainerType::iterator> eventIndexMap;

private:
    std::shared_ptr<smUnifiedId> uniqueId; ///< unique Id
    std::shared_ptr<smRenderDetail> renderDetail; ///< specifies visualization type
    smClassDrawOrder drawOrder; ///< draw order of the object
};

/// \brief for future use
class smInterface : public smCoreClass
{

};

#endif
