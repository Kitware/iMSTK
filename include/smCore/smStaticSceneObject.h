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

#ifndef SMSTATICSCENEOBJECT_H
#define SMSTATICSCENEOBJECT_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smCore/smSceneObject.h"
#include "smMesh/smSurfaceMesh.h"
#include "smCore/smCoreClass.h"

/// \brief static scene object
class smStaticSceneObject: public smSceneObject
{
public:
    /// \brief static scene object contains a mesh
    smSurfaceMesh *mesh;
    /// \brief constructor receives the error log
    smStaticSceneObject(smErrorLog *p_log = NULL);
    ~smStaticSceneObject();

    //not implemented yet..tansel
    virtual void serialize(void *p_memoryBlock);

    //not implemented yet..tansel
    virtual void unSerialize(void *p_memoryBlock);

    ///not implemented yet.
    virtual smSceneObject *clone();

    /// \brief Initialization routine
    virtual void init();

    /// \brief called if the object is added to the viewer.
    virtual void draw(const smDrawParam &p_params);
};

#endif
