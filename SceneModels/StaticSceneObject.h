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

#ifndef SMSTATICSCENEOBJECT_H
#define SMSTATICSCENEOBJECT_H

// SimMedTK includes
#include "Core/Config.h"
#include "Core/Model.h"
#include "SceneModels/SceneObject.h"
#include "Core/CoreClass.h"
#include "Core/Model.h"
#include "Mesh/SurfaceMesh.h"

class ErrorLog;

namespace core {
    class Event;
}

///
/// \brief This type of models are meant to be static in the sense that dynamics do not
///     apply to them. They can be used to model objects that do not move in the scene or
///     or objects that are controlled by external hardware, i.e. haptics devices.
///
class StaticSceneObject : public SceneObject
{
public:
    ///
    /// \brief Constructor
    ///
    StaticSceneObject(std::shared_ptr<ErrorLog> p_log = nullptr);

    ///
    /// \brief Destructor
    ///
    ~StaticSceneObject() = default;

    ///
    /// \brief Initialize this model.
    ///
    void initialize() override;

    ///
    ///
    ///
    void printInfo() const override;

    ///
    ///
    ///
    virtual void handleEvent(std::shared_ptr<core::Event>) override {}

    ///
    ///
    ///
    void update(const double /*dt*/) override
    {}

    ///
    /// \brief Initialize mesh for this model
    ///
    void loadMesh(const std::string &file)
    {
        this->fileName = file;
        this->loadInitialStates();
    }

private:
    ///
    /// \brief Initialize the model for this scene model.
    ///
    void loadInitialStates() override;

    ///////////////////////////////////////////////////////////////////////////////
    ///////////                 DO NOT USE THIS API                     ///////////
    ///////////////////////////////////////////////////////////////////////////////
    //////////// TODO: These are pure virtual methods from superclass. ////////////
    ////////////    They should be removed in the future.              ////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///
    ///not implemented yet.
    ///
    virtual std::shared_ptr<SceneObject> clone() override { return nullptr; }

    ///
    //not implemented yet..tansel
    ///
    virtual void serialize(void */*p_memoryBlock*/) override {}

    ///
    //not implemented yet..tansel
    ///
    virtual void unSerialize(void */*p_memoryBlock*/) override {}

private:
    std::string fileName;
};

#endif
