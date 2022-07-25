/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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