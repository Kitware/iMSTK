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

#ifndef SMUNIFIEDID_H
#define SMUNIFIEDID_H

// STL includes
#include <atomic>
#include <memory>


/// \brief  This class creates a unique unified id of each object in the framework
struct UnifiedId
{
public:
    typedef std::shared_ptr<UnifiedId> Pointer;

public:
    /// \brief  constructor
    UnifiedId();

    /// \brief comparison with another std::shared_ptr<UnifiedId>
    bool operator==(const UnifiedId *id);

    /// \brief comparison with id
    bool operator!=(const short &id);

    /// Accessor

    inline const short &getId() const
    {
        return this->ID;
    }

private:
    static std::atomic_int IDcounter; // atomic integer counter that is used to assign a unique number for  each object
    short ID; //  unique ID

    UnifiedId(const UnifiedId&) = delete;
    UnifiedId &operator=(const UnifiedId&) = delete;
};

#endif // SMUNIFIEDID_H
