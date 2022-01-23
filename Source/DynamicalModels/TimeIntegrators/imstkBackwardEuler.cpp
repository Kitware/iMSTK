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

#include "imstkBackwardEuler.h"

namespace imstk
{
void
BackwardEuler::updateStateGivenDv(std::shared_ptr<FeDeformBodyState> prevState,
                                  std::shared_ptr<FeDeformBodyState> currentState,
                                  Vectord&                           dV)
{
    currentState->getQDot() = prevState->getQDot() + dV;
    currentState->getQ()    = prevState->getQ() + m_dT * currentState->getQDot();
}

void
BackwardEuler::updateStateGivenDu(std::shared_ptr<FeDeformBodyState> prevState,
                                  std::shared_ptr<FeDeformBodyState> currentState,
                                  Vectord&                           dU)
{
    currentState->getQ()    = prevState->getQ() + dU;
    currentState->getQDot() = (currentState->getQ() - prevState->getQ()) / m_dT;
}

void
BackwardEuler::updateStateGivenV(std::shared_ptr<FeDeformBodyState> prevState,
                                 std::shared_ptr<FeDeformBodyState> currentState,
                                 Vectord&                           v)
{
    currentState->getQDot() = v;
    currentState->getQ()    = prevState->getQ() + m_dT * currentState->getQDot();
}

void
BackwardEuler::updateStateGivenU(std::shared_ptr<FeDeformBodyState> prevState,
                                 std::shared_ptr<FeDeformBodyState> currentState,
                                 Vectord&                           u)
{
    currentState->getQ()    = u;
    currentState->getQDot() = (currentState->getQ() - prevState->getQ()) / m_dT;
}
} // namespace imstk