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

#ifndef VTKMESHREADER_H
#define VTKMESHREADER_H

#include "InputOutput/MeshIO.h"
#include "Core/Factory.h"

class VTKMeshReader : MeshIO
{
public:
    VTKMeshReader()
    {
        auto delegate = Factory<ReaderDelegate>::createSubclassForGroup(
            "VTKMeshRenderDelegate",MeshFileType::VTK);
        this->setReaderDelegate(delegate);
    }
    ~VTKMeshReader();



private:
    class Reader;
    Reader *reader;
};

#endif // VTKMESHREADER_H
