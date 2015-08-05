/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */


#include "Core/Factory.h"
#include "InputOutput/ReaderDelegate.h"
#include "Mesh//VegaVolumetricMesh.h"

// Vega includes
#include "volumetricMesh.h"
#include "cubicMesh.h"
#include "tetMesh.h"

class VegaMeshReaderDelegate : ReaderDelegate
{
public:
    void read()
    {
        // const cast to silence warnings later on
        auto name = const_cast<char*>(this->meshIO->getFileName().c_str());

        std::shared_ptr<VolumetricMesh> mesh = std::make_shared<VolumetricMesh>();

        VolumetricMesh::elementType elementType = VolumetricMesh::getElementType(name);
        switch(elementType)
        {
            case VolumetricMesh::TET:
            {
                mesh = std::make_shared<TetMesh>(name, true);
                break;
            }
            case VolumetricMesh::CUBIC:
            {
                mesh = std::make_shared<CubicMesh>(name, true);
                break;
            }
            default:
            {
                mesh.reset();
                std::cerr << "Unknown element type." << std::endl;
            }
        }

        auto volumetricMesh = std::make_shared<VegaVolumetricMesh>(true);
        volumetricMesh->setVegaMesh(mesh);
        this->meshIO->setMesh(volumetricMesh);
    }
};

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
SIMMEDTK_BEGIN_ONLOAD(register_vega_mesh_reader_delegate)
SIMMEDTK_REGISTER_CLASS(ReaderDelegate, ReaderDelegate, VegaMeshReaderDelegate, ReaderDelegate::ReaderGroup::Vega);
SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
