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

#ifndef SMMATH_H
#define SMMATH_H
#include "smConfig.h"
#include "smCoreClass.h"

/// \brief  generic definitions
#define SM_PI           3.141592653589793
#define SM_PI_HALF      1.57079632679489661923
#define SM_PI_QUARTER   0.78539816339744830961
#define SM_PI_TWO       6.28318530717958647692
#define SM_PI_INV       0.31830988618379067154


#define SM_MATRIX_PRECISION    0.000000001
#define SM_DEGREES2RADIANS(X)   X*0.0174532925
#define SM_RADIANS2DEGREES(X)   X*57.2957795

/// \brief generic math class
class smMath: smCoreClass
{
public:

    smMath();
    /// \brief computes power
    static smInt pow(smInt p_base, smInt p_pow);
    /// \brief linear interpolation with min, max
    static smFloat interpolate(smInt current, smInt min, smInt max);
};
/// \brief hash function
template<class T>
class smBaseHash: public smCoreClass
{
public:
    smBaseHash()
    {
    }
    /// \brief  computes hash based on the has table size, given one number x
    template<T>
    inline smUInt computeHash(smUInt p_tableSize, T p_x);

    /// \brief  computes hash based on the has table size, given  numbers x,y
    template<T>
    inline smUInt computeHash(smUInt p_tableSize, T p_x, T p_y);
    /// \brief  computes hash based on the has table size, given  numbers x,y,z
    template<T>
    inline smUInt computeHash(smUInt p_tableSize, T p_x, T p_y, T p_z);
};

template<class T>
class smCollisionHash: public smBaseHash<T>
{

};

#endif
