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

#ifndef SMTK_EXAMPLES_COMMON_HAPTICCONTROLLER_H_
#define SMTK_EXAMPLES_COMMON_HAPTICCONTROLLER_H_

#include "Core/CoreClass.h"
#include "Event/HapticEvent.h"
#include "Simulators/VegaFemSceneObject.h"

namespace mstk {
namespace Examples {
namespace Common {

/// \brief Will shutdown the SDK when the a specified key is pressed
class hapticController : public smCoreClass
{
public:
    /// \brief Default constructor
    ///
    /// \detail Default key is escape
    hapticController(){}

    /// \brief Event handling function from smCoreClass
    ///
    /// \param event Event to handle from the main event system
    void handleEvent(std::shared_ptr<mstk::Event::smEvent> event) override;

    /// \brief Set the scene objects which gets affected by this haptic event
    ///
    /// \param scene object of type smVegaFemSceneObject
    void setVegaFemSceneObject(const std::shared_ptr<smVegaFemSceneObject> &femSO);

private:
    std::shared_ptr<smVegaFemSceneObject> femSceneObject; ///< Pointer to the vegafemscene object
};

}//Common
}//Examples
}//tk

#endif
