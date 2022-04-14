/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#pragma once

#include "imstkCDObjectFactory.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace imstk
{
class VisualModel;
class VTKRenderDelegate;

///
/// \class RenderDelegateObjectFactory
///
/// \brief This is the factory class for VTKRenderDelegates.
///
/// VTKRenderDelegate's are constructed with a VisualModel.
///
/// The factory implements its own creation scheme/logic, but if a
/// delegateHint is provided in the VisualModel it will search for a
/// creation function by that name.
///
/// Note: Does not auto register VTKRenderDelegate's. If one creates
/// their own VTKRenderDelegate they must register themselves.
///
class VTKRenderDelegate;

class RenderDelegateObjectFactory : public ObjectFactory<std::shared_ptr<VTKRenderDelegate>, std::shared_ptr<VisualModel>>
{
public:
    static std::shared_ptr<VTKRenderDelegate> makeRenderDelegate(std::shared_ptr<VisualModel> visualModel);
};

template<typename T>
using RenderDelegateRegistrar = SharedObjectRegistrar<VTKRenderDelegate, T, std::shared_ptr<VisualModel>>;

#define IMSTK_REGISTER_RENDERDELEGATE(geomType, objType) RenderDelegateRegistrar<objType> _imstk_registerrenderdelegate ## geomType(#geomType);
} // namespace imstk