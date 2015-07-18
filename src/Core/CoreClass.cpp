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

std::shared_ptr<smtk::Event::smEventHandler>
smCoreClass::eventHandler = std::make_shared<smtk::Event::smEventHandler>();

smCoreClass::smCoreClass() :
    name(""), listening(false)
{
    drawOrder = SIMMEDTK_DRAW_BEFOREOBJECTS;
    uniqueId = std::make_shared<smUnifiedId>();
    renderDetail = std::make_shared<smRenderDetail>();
}

smCoreClass::smCoreClass(const std::string &_name) :
    name(_name), listening(false)
{
    drawOrder = SIMMEDTK_DRAW_BEFOREOBJECTS;
    uniqueId = std::make_shared<smUnifiedId>();
    renderDetail = std::make_shared<smRenderDetail>();
}

const smClassType &smCoreClass::getType() const
{
    return type;
}

void smCoreClass::setType(const smClassType& newType)
{
    this->type = newType;
}

void smCoreClass::initSimulate( const smSimulationParam &/*p_params*/ )
{
    std::cout << "Default initSimulate" << std::endl;
}

void smCoreClass::simulate( const smSimulationParam &/*p_params*/ )
{
    std::cout << "Default simulate" << std::endl;
}

void smCoreClass::print() const
{
    std::cout << "Default print" << std::endl;
}

void smCoreClass::handleEvent(std::shared_ptr< smtk::Event::smEvent > event)
{
    std::cout << "Default handleEvent" << std::endl;
    std::cout << "Sender " << int(event->getSender()) << std::endl;
    std::cout << "Priority " << int(event->getPriority()) << std::endl;
    std::cout << "Type " << int(smtk::Event::smEvent::EventName) << std::endl;
}

void smCoreClass::setName( const std::string &p_objectName )
{
    name = p_objectName;
}

const std::string &smCoreClass::getName() const
{
    return name;
}

/// \brief Get render delegate
std::shared_ptr<smRenderDelegate> smCoreClass::getRenderDelegate() const
{
  return this->renderDelegate;
}

/// \brief Set the delegate used to render this instance
void smCoreClass::setRenderDelegate(std::shared_ptr<smRenderDelegate> delegate)
{
  this->renderDelegate = delegate;
  if (this->renderDelegate)
    this->renderDelegate->setSourceGeometry(this);
}

void smCoreClass::initDraw()
{
  if (this->renderDelegate)
    this->renderDelegate->initDraw();
}

void smCoreClass::draw() const
{
  if (this->renderDelegate)
    this->renderDelegate->draw();
}
