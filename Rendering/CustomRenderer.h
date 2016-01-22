// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef RENDERING_CUSTOMRENDERER_H
#define RENDERING_CUSTOMRENDERER_H


// iMSTK includes
#include "Core/ConfigRendering.h"
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "SceneModels/SceneObject.h"

//forward declaration
class SceneObject;

/// \brief custom renderer for the object.
///you could draw with one custom renderer with multiple objects.
///This is the reason why the functions are taken objects.
class CustomRenderer: public CoreClass
{

public:
    /// \briefthis function should be implemented based on objects
    //virtual void init();
    /// \brief draw pre, default and post draw routines
    virtual void preDrawObject(const SceneObject &p_object);
    virtual void drawObject(const SceneObject &p_object);
    virtual void postDrawObject(const SceneObject &p_object);
    virtual void draw() const;
};

/// \briefderive this class if you want to render a class in the viewer.
class CustomRenderInterface
{
    virtual void draw();
};

#endif
