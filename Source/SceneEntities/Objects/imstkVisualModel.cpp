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

#include "imstkVisualModel.h"
#include "imstkRenderMaterial.h"

namespace imstk
{
VisualModel::VisualModel(std::shared_ptr<Geometry> geometry) : m_geometry(geometry), m_renderMaterial(std::make_shared<RenderMaterial>())
{
}

VisualModel::VisualModel(std::shared_ptr<Geometry>       geometry,
                         std::shared_ptr<RenderMaterial> renderMaterial) :
    m_geometry(geometry),
    m_renderMaterial(renderMaterial)
{
}

VisualModel::VisualModel() : m_renderMaterial(std::make_shared<RenderMaterial>())
{
}

void
VisualModel::setRenderMaterial(std::shared_ptr<RenderMaterial> renderMaterial)
{
    m_renderMaterial = renderMaterial;
    this->postModified();
}

void
VisualModel::setIsVisible(const bool visible)
{
    m_isVisible = visible;
    this->postModified();
}

bool
VisualModel::getRenderDelegateCreated(Renderer* ren)
{
    if (m_renderDelegateCreated.count(ren) == 0)
    {
        m_renderDelegateCreated[ren] = false;
        return false;
    }
    else
    {
        return m_renderDelegateCreated[ren];
    }
}
}
