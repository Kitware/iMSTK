/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
    VTKTextureDelegate(std::shared_ptr<Texture> texture);
    ~VTKTextureDelegate() override = default;

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
} // namespace imstk
