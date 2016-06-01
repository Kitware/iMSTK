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

#ifndef imstkKinematicState_h
#define imstkKinematicState_h


namespace imstk
{

///
/// \class KinematicState
///
/// \brief Kinematic state of the body in time
///
template<class T>
class KinematicState
{
public:

    ///
    /// \brief Constructor
    ///
    KinematicState() = default;
    KinematicState(const T& u, const T& v);

    ///
    /// \brief Set the state to a given one
    ///
    void setState(const T& u, const T& v);

    ///
    /// \brief Destructor
    ///
    ~KinematicState(){}

protected:
    T m_displacement;
    T m_velocity;
};

}

#endif // ifndef imstkKinematicState_h
