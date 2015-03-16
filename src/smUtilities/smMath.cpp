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

// SimMedTK includes
#include "smUtilities/smMath.h"

smMath::smMath()
{
    type = SIMMEDTK_SMMATH;
}
smInt smMath::pow(smInt p_base, smInt p_pow)
{
    smInt res = 1;

    for (smInt i = 0; i < p_pow; i++)
    {
        res *= p_base;
    }

    return res;
}
smFloat smMath::interpolate(smInt current, smInt min, smInt max)
{
    if (current < min)
    {
        return 0.0;
    }
    else if (current > max)
    {
        return 1.0;
    }
    else
    {
        return (smFloat)(current - min) / (smFloat)(max - min);
    }
}
