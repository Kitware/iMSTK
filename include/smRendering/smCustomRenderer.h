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

#ifndef SMCUSTOMRENDERER_H
#define SMCUSTOMRENDERER_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smRendering/smConfigRendering.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"

//forward declaration
class smSceneObject;

/// \brief custom renderer for the object.
///you could draw with one custom renderer with multiple objects.
///This is the reason why the functions are taken objects.
class smCustomRenderer: public smCoreClass
{

public:
    /// \briefthis function should be implemented based on objects
    //virtual void init();
    /// \brief draw pre, default and post draw routines
    virtual void preDraw(const smSceneObject &p_object);
    virtual void draw(const smDrawParam &p_object);
    virtual void draw(const smSceneObject &p_object);
    virtual void postDraw(const smSceneObject &p_object);
};

/// \briefderive this class if you want to render a class in the viewer.
class smCustomRenderInterface: public smInterface
{

    virtual void draw();
};

#endif
