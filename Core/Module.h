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

#ifndef CORE_MODULE_H
#define CORE_MODULE_H

// STL includes
#include <memory>

// iMSTK includes
#include "Core/CoreClass.h"

namespace imstk {

class Scene;
class Dispatcher;

/// \brief Abstract base class. Each module runs on its own thread.
/// Every other thread should derive this class.
class Module: public CoreClass
{
public:
    ///
    /// \brief Constructor initializes the module.
    ///
    Module();
    virtual ~Module();

    ///
    /// \brief Set termination flags to true.
    ///
    void terminate();

    ///
    /// \brief Check if the termination of the module is completed.
    ///
    bool isTerminationDone();

    ///
    /// \brief Check if the termination signal has being sent.
    ///
    bool isTerminated();

    ///
    /// \brief Wait for termination.
    ///
    void waitTermination();

    ///
    /// \brief Get module id.
    ///
    short int getModuleId();

    ///
    /// \brief Call are made for begin module and end module before and after each frame.
    ///
    virtual void beginModule();
    virtual void endModule();

    ///
    /// \brief Set SceneList
    ///
    void setSceneList(const std::vector<std::shared_ptr<Scene>> &newSceneList);

    ///
    /// \brief Get SceneList
    ///
    const std::vector<std::shared_ptr<Scene>> &getSceneList() const;

    ///
    /// \brief Module initialization routine.
    ///
    virtual bool init() = 0;

    ///
    /// \brief Begin frame will be called before the cycle.
    ///
    virtual void beginFrame() = 0;

    ///
    /// \brief End frame will be called after the cycle.
    ///
    virtual void endFrame() = 0;

    ///
    /// \brief Execution function. Main module execution.
    ///
    virtual void exec() = 0;

protected:
    bool isInitialized;                     //!< Initialization flag
    bool terminateExecution;                //!< Execution termination flag
    bool terminationCompleted;              //!< True when the terminatation is completed
    std::shared_ptr<Dispatcher> dispathcer; //!< Dispatcher references
    std::vector<std::shared_ptr<Scene>> sceneList; //!< Scene list in the environment

};

}

#endif
