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

#ifndef READERDELEGATE_H
#define READERDELEGATE_H

#include <cstdint>
#include <memory>

#include "InputOutput/IOMesh.h"
#include "Core/Config.h"

class IOMeshDelegate
{
public:
    typedef std::shared_ptr<IOMeshDelegate> Ptr;

public:
    setSource(std::shared_ptr<IOMesh> src)
    {
        meshIO = src;
    }

    virtual void read(){ }
    virtual void write(){ }

protected:
    std::shared_ptr<IOMesh> meshIO;
};
#endif // READERDELEGATE_H
