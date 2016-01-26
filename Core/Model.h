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

#ifndef CORE_MODEL_H
#define CORE_MODEL_H

// STL includes
#include <memory>

// iMSTK includes
#include "Core/CoreClass.h"
#include "Core/RenderDelegate.h"

namespace imstk {

class BaseMesh;
class CoreClass;

class Model
{
public:
    Model();
    ~Model();

    virtual std::shared_ptr<BaseMesh> getMesh();
    virtual std::shared_ptr<CoreClass> getObject();
    virtual void draw();

    virtual void setRenderDelegate(RenderDelegate::Ptr delegate);

    ///
    /// \brief Get render delegate
    ///
    RenderDelegate::Ptr getRenderDelegate() const;

private:
    RenderDelegate::Ptr renderDelegate;
};

}

#endif // CORE_MODEL
