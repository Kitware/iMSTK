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
#include "Math.h"

smMath::smMath()
{
    type = core::ClassType::Math;
}

int smMath::pow(int p_base, int p_pow)
{
    int res = 1;

    for (int i = 0; i < p_pow; i++)
    {
        res *= p_base;
    }

    return res;
}

float smMath::interpolate(int current, int min, int max)
{
    if (current < min)
    {
        return 0.0;
    }
    else if (current > max)
    {
        return 1.0;
    }
    return (current - min) / (max - min);
}
