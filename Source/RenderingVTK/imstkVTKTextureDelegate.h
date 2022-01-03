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

#include "imstkTextureDelegate.h"

#include <vtkSmartPointer.h>

class vtkTexture;

namespace imstk
{
class VTKTextureDelegate;

///
/// \class VTKTextureDelegate
///
/// \brief couples a imstk texture to a VTK texture
///
class VTKTextureDelegate : public TextureDelegate
{
public:
    ///
    /// \brief Default constructor
    ///
    VTKTextureDelegate(std::shared_ptr<Texture> texture);

    virtual ~VTKTextureDelegate() override = default;

    ///
    /// \brief Gets the VTK texture coupled to the imstk texture
    ///
    /// \returns VTK texture
    ///
    vtkSmartPointer<vtkTexture> getVtkTexture() const { return m_vtkTexture; }

    ///
    /// \brief Get the imstk texture
    ///
    std::shared_ptr<Texture> getTexture() const { return m_texture; }

    ///
    /// \brief Return the VTK texture name
    ///
    /// \returns VTK texture
    ///
    const std::string& getTextureName() const { return m_textureName; }

protected:
    void textureModified(Event* e);

    vtkSmartPointer<vtkTexture> m_vtkTexture; ///< VTK texture
    std::shared_ptr<Texture>    m_texture;    ///< iMSTK texture
    std::string m_textureName;                ///< VTK texture unique name
};
}
