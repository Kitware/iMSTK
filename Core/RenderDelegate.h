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

#ifndef CORE_RENDERDELEGATE_H
#define CORE_RENDERDELEGATE_H

#include "Core/RenderDetail.h" // for GeometrySource

#include <memory> // for shared_ptr

namespace imstk {

class RenderDelegate
{
public:

    enum RendererType
    {
        VTK,
        Other
    };

    typedef std::shared_ptr<RenderDelegate> Ptr;

    virtual void initDraw()
    { }
    virtual void draw() const
    { }
    virtual void modified()
    { }

    virtual bool isTargetTextured() const
    {
        return false;
    }

    template<typename T>
    void setSourceGeometry ( T* srcGeom )
    {
        this->sourceGeometry.setSource ( srcGeom );
    }

    template<typename T>
    T* getSourceGeometryAs() const
    {
        return this->sourceGeometry.sourceAs<T>();
    }
protected:
    GeometrySource sourceGeometry; // object to render when draw() is called
    std::string type;
};

}

#endif // CORE_RENDERABLE_H

