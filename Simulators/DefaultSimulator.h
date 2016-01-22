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

#ifndef SIMULATORS_DUMMYSIMULATOR_H
#define SIMULATORS_DUMMYSIMULATOR_H

#include <memory>
#include <vector>
#include <functional>

// iMSTK includes
#include "Core/Config.h"
#include "Simulators/ObjectSimulator.h"
#include "Core/ErrorLog.h"
#include "Core/Vector.h"

class ToolCoupler;
class MeshModel;
///
/// \brief Default simulator that applies operations to the position array of the
///     undelying scene object model.
///     Operations can be added to the list of operator using \addOperation.
///     This function takes a lambda with void(std::vector<core::Vec3d>&) signature.
///
class DefaultSimulator: public ObjectSimulator
{
    typedef std::function<void(std::shared_ptr<MeshModel> &sceneObject)> OperationType;
public:
    /// \brief Constructor/Destructor
    DefaultSimulator();
    ~DefaultSimulator(){}

    void addOperation(const OperationType &op)
    {
        this->operatorFunctions.push_back(op);
    }

protected:
    ///
    /// Overriden methods
    ///
    virtual void beginExecution() override;
    virtual void initialize() override;
    virtual void run() override;
    void endExecution() override;
    void syncBuffers() override;
    void handleEvent(std::shared_ptr<core::Event> p_event) override;

    void updateHapticForces(std::shared_ptr<SceneObject> sceneObject);

private:
    std::vector<OperationType> operatorFunctions;
    std::shared_ptr<ToolCoupler> hapticTool;

};

#endif
