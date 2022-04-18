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

#include "imstkFactory.h"

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
/// \brief Manages and generates the VTKRenderdelegates for all VisualModels.
///
/// The factory is a singleton and can be accessed anywhere.
/// Given a visual model this will, if available generate a renderdelegate that
/// may be able to render the model. \sa VisualModel::delegateHint() is used to
/// determine what delegate should be returned. delegateHint() has some functionality
/// to determine a default Hint and can be overridden by the user.
/// The generation Will fail if the name is not known to the factory
///
/// There are multiple ways to register a renderdelegate
/// \code
/// IMSTK_REGISTER_RENDERDELEGATE(geometryType, delegateType)
/// \endcode
/// will register the delegate for the class-name of the geometry,
/// this will satisfy the default mechanism
/// If a custom delegate is wanted this form may be preferable
/// \code
/// RenderDelegateRegistrar<delegateType> registrar("HintName");
/// \endcode
///
class VTKRenderDelegate;

class RenderDelegateObjectFactory : public ObjectFactory<std::shared_ptr<VTKRenderDelegate>, std::shared_ptr<VisualModel>>
{
public:
    ///
    /// \brief attempt to create a delegate for the given visual model
    /// \param visualModel the model we need a delegate for
    static std::shared_ptr<VTKRenderDelegate> makeRenderDelegate(std::shared_ptr<VisualModel> visualModel);
};

/// \brief class for automatically registering a delegate
/// \tparam T type of the delegate object to register
template<typename T>
using RenderDelegateRegistrar = SharedObjectRegistrar<VTKRenderDelegate, T, std::shared_ptr<VisualModel>>;

#define IMSTK_REGISTER_RENDERDELEGATE(geomType, objType) RenderDelegateRegistrar<objType> _imstk_registerrenderdelegate ## geomType(#geomType);
} // namespace imstk