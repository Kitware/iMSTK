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
class RenderDelegateObjectFactory
{
public:
    using DelegateMakeFunc = std::function<std::shared_ptr<VTKRenderDelegate>(std::shared_ptr<VisualModel>)>;

    ///
    /// \brief Register the RenderDelegate creation function with
    /// template type. Provide a delegateHint in the VisualModel to use
    /// this creation function instead. Creation functions can be overriden
    ///
    template<typename T>
    static void registerDelegate(std::string name)
    {
        static_assert(std::is_base_of<VTKRenderDelegate, T>::value,
            "T must be a subclass of VTKRenderDelegate");
        m_objCreationMap[name] = [](std::shared_ptr<VisualModel> visualModel)
                                 {
                                     return std::make_shared<T>(visualModel);
                                 };
    }

    ///
    /// \brief Creates a VTKRenderDelegate object by VisualModel if registered to factory
    ///
    static std::shared_ptr<VTKRenderDelegate> makeRenderDelegate(std::shared_ptr<VisualModel> visualModel);

private:
    static std::unordered_map<std::string, DelegateMakeFunc> m_objCreationMap;
};
#define REGISTER_RENDER_DELEGATE(delegateType) RenderDelegateObjectFactory::registerDelegate<delegateType>(#delegateType)
} // namespace imstk