/*=========================================================================

  Library: iMSTK

  Copyright (c) Kitware, Inc.

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

#ifndef imstkModule_h
#define imstkModule_h

#include <iostream>

namespace imstk {

enum class ModuleStatus
{
    INACTIVE,
    RUNNING,
    PAUSED,
    TERMINATING
};

class Module
{
public:
    ~Module() = default;

    void exec();
    void run();
    void pause();
    void terminate();

    const ModuleStatus& getStatus() const;
    const std::string& getName() const;

protected:
    Module(std::string name) : m_name(name) {}

    virtual void initModule() = 0;
    virtual void runModule() = 0;
    virtual void cleanUpModule() = 0;

    ModuleStatus m_status = ModuleStatus::INACTIVE;
    std::string m_name;
};

}

#endif
