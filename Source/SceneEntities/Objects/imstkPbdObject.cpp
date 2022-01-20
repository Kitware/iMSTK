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

#include "imstkPbdObject.h"
#include "imstkLogger.h"
#include "imstkPbdModel.h"

namespace imstk
{
std::shared_ptr<PbdModel>
PbdObject::getPbdModel()
{
    m_pbdModel = std::dynamic_pointer_cast<PbdModel>(m_dynamicalModel);
    return m_pbdModel;
}

bool
PbdObject::initialize()
{
    m_pbdModel = std::dynamic_pointer_cast<PbdModel>(m_dynamicalModel);
    if (m_pbdModel == nullptr)
    {
        LOG(FATAL) << "Dynamics pointer cast failure in PbdObject::initialize()";
        return false;
    }

    DynamicObject::initialize();

    return true;
}
}