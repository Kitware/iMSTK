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

#ifndef SMIMPORTEXPORT_H
#define SMIMPORTEXPORT_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smMesh.h"

/// \brief
enum smExportOptions
{
    SIMMEDTK_EXPORT_ALL
};

/// \brief class for importing and exporting meshes
class smImportExport: public smCoreClass
{

public:
    static smBool convertToJSON(smMesh *mesh, const smString& outputFileName,
                                smExportOptions p_export = SIMMEDTK_EXPORT_ALL);
};

#endif
