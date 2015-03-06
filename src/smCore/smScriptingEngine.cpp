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

#include <iostream>
#include <string>
#include "smCore/smScriptingEngine.h"

void smScriptingEngine::list()
{

    std::cout << "List of Registered Variables" << "\n";
    for(auto& x: registeredVariables)
    {
        std::cout << x.second->className << "-->" << x.second->variableName << "\n";
    }

    std::cout << "List of Registered Classes" << "\n";

    for(auto& x: registeredClasses)
        std::cout << x.second->getName() << "\n";
}
