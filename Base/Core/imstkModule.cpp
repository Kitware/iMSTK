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

#include "imstkModule.h"

#include <iostream>

namespace imstk {

void Module::exec()
{
    m_status = ModuleStatus::RUNNING;
    this->initModule();
    while( m_status !=  ModuleStatus::TERMINATING )
    {
      if( m_status == ModuleStatus::RUNNING )
      {
          this->runModule();
      }
    }
    this->cleanUpModule();
    m_status = ModuleStatus::INACTIVE;
}

void Module::run()
{
    switch(m_status)
    {
    case ModuleStatus::PAUSED :
        m_status = ModuleStatus::RUNNING;
        break;
    case ModuleStatus::RUNNING :
        std::cerr << "Can not run " << m_name <<std::endl
                  << "Module already running." <<std::endl;
        break;
    default :
        std::cerr << "Can not run " << m_name <<std::endl
                  << "Module terminating or not active."
                  << "Run `module::exec()` to launch your module." <<std::endl;
        break;
    }
}

void Module::pause()
{
    switch(m_status)
    {
    case ModuleStatus::RUNNING :
        m_status = ModuleStatus::PAUSED;
        break;
    case ModuleStatus::PAUSED :
        std::cerr << "Can not pause " << m_name <<std::endl
                  << "Module already running." <<std::endl;
        break;
    default :
        std::cerr << "Can not pause " << m_name <<std::endl
                  << "Module terminating or not active."
                  << "Run `module::exec()` to launch your module." <<std::endl;
        break;
    }
}

void Module::terminate()
{
    switch(m_status)
    {
    case ModuleStatus::TERMINATING :
        std::cerr << "Can not terminate " << m_name <<std::endl
                  << "Module already terminating." <<std::endl;
        break;
    case ModuleStatus::INACTIVE :
        std::cerr << "Can not terminate " << m_name <<std::endl
                  << "Module not active."
                  << "Run `module::exec()` to launch your module." <<std::endl;
        break;
    default :
        m_status = ModuleStatus::TERMINATING;
        break;
    }
}

const ModuleStatus& Module::getStatus() const
{
    return m_status;
}

const std::string& Module::getName() const
{
    return m_name;
}

}
