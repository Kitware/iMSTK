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

#include <memory>
#include <InputOutput/MeshIO.h>

class ReaderDelegate
{
public:
    typedef std::shared_ptr<ReaderDelegate> Ptr;

    virtual void read(){ }

    template<typename T>
    void setReader ( T* src )
    {
        this->reader = src;
    }

    template<typename T>
    std::shared_ptr<T> getReaderAs() const
    {
        auto child = std::dynamic_pointer_cast<T>(this->reader);
        if(child)
        {
            return child;
        }

        return nullptr;
    }

protected:
    std::shared_ptr<MeshIO> reader; // object to render when draw() is called
};
#endif // READERDELEGATE_H
