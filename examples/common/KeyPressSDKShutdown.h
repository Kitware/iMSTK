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

#ifndef SMTK_EXAMPLES_COMMON_KEYPRESSSDKSHUTDOWN_H
#define SMTK_EXAMPLES_COMMON_KEYPRESSSDKSHUTDOWN_H

#include "Core/CoreClass.h"
#include "Event/Key.h"

namespace mstk {
namespace Examples {
namespace Common {

/// \brief Will shutdown the SDK when the a specified key is pressed
class KeyPressSDKShutdown : public CoreClass
{
public:
    /// \brief Default constructor
    ///
    /// \detail Default key is escape
    KeyPressSDKShutdown();

    /// \brief Event handling function from CoreClass
    ///
    /// \param event Event to handle from the main event system
    void handleEvent(std::shared_ptr<mstk::Event::Event> event) override;

    /// \brief Set the keyboard key to listen for
    ///
    /// \param key The keyboard key to listen for
    void setKey(mstk::Event::smKey key);

private:
    mstk::Event::smKey key; ///< When this key is pressed, shutdown the framework
};

}//Common
}//Examples
}//tk

#endif
