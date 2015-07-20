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

#include "CoreClass.h"

#include "EventHandler.h"
#include "RenderDelegate.h"

std::shared_ptr<mstk::Event::smEventHandler>
CoreClass::eventHandler = std::make_shared<mstk::Event::smEventHandler>();

CoreClass::CoreClass() :
    name(""), listening(false)
{
    drawOrder = core::ClassDrawOrder::BeforeObjects;
    uniqueId = std::make_shared<UnifiedId>();
    renderDetail = std::make_shared<RenderDetail>();
}

CoreClass::CoreClass(const std::string &_name) :
    name(_name), listening(false)
{
    drawOrder = core::ClassDrawOrder::BeforeObjects;
    uniqueId = std::make_shared<UnifiedId>();
    renderDetail = std::make_shared<RenderDetail>();
}

const core::ClassType &CoreClass::getType() const
{
    return type;
}

void CoreClass::setType(const core::ClassType& newType)
{
    this->type = newType;
}

void CoreClass::initSimulate( const smSimulationParam &/*p_params*/ )
{
    std::cout << "Default initSimulate" << std::endl;
}

void CoreClass::simulate( const smSimulationParam &/*p_params*/ )
{
    std::cout << "Default simulate" << std::endl;
}

void CoreClass::print() const
{
    std::cout << "Default print" << std::endl;
}

void CoreClass::handleEvent(std::shared_ptr< mstk::Event::Event > event)
{
    std::cout << "Default handleEvent" << std::endl;
    std::cout << "Sender " << int(event->getSender()) << std::endl;
    std::cout << "Priority " << int(event->getPriority()) << std::endl;
    std::cout << "Type " << int(mstk::Event::Event::EventName) << std::endl;
}

void CoreClass::setName( const std::string &p_objectName )
{
    name = p_objectName;
}

const std::string &CoreClass::getName() const
{
    return name;
}

/// \brief Get render delegate
std::shared_ptr<RenderDelegate> CoreClass::getRenderDelegate() const
{
  return this->renderDelegate;
}

/// \brief Set the delegate used to render this instance
void CoreClass::setRenderDelegate(std::shared_ptr<RenderDelegate> delegate)
{
  this->renderDelegate = delegate;
  if (this->renderDelegate)
    this->renderDelegate->setSourceGeometry(this);
}

void CoreClass::initDraw()
{
  if (this->renderDelegate)
    this->renderDelegate->initDraw();
}

void CoreClass::draw() const
{
  if (this->renderDelegate)
    this->renderDelegate->draw();
}
