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

#ifndef imstkRenderMaterial_h
#define imstkRenderMaterial_h

#include "imstkTexture.h"
#include "imstkColor.h"
#include "imstkTextureManager.h"

#include <memory>
#include <vector>

#include "g3log/g3log.hpp"

namespace imstk
{
class RenderMaterial
{
public:
    enum DisplayMode
    {
        SURFACE,
        WIREFRAME,
        POINTS,
        WIREFRAME_SURFACE
    };

    ///
    /// \brief Constructor
    ///
    RenderMaterial();

    ///
    /// \brief Get/Set display mode
    ///
    const DisplayMode getDisplayMode() const;
    void setDisplayMode(const DisplayMode displayMode);

    ///
    /// \brief Get/Set line width or the wireframe
    ///
    const float getLineWidth() const;
    void setLineWidth(const float width);

    ///
    /// \brief Get/Set point size
    ///
    const float getPointSize() const;
    void setPointSize(const float size);

    ///
    /// \brief Backface culling on/off
    ///
    const bool getBackFaceCulling() const;
    void setBackFaceCulling(const bool culling);
    void backfaceCullingOn();
    void backfaceCullingOff();

    ///
    /// \brief Get/Set the diffuse color
    ///
    const Color& getDiffuseColor() const;
    void setDiffuseColor(const Color color);

    ///
    /// \brief Get/Set the specular color (only set for metals)
    ///
    const Color& getSpecularColor() const;
    void setSpecularColor(const Color color);

    ///
    /// \brief Get/Set the specularity
    ///
    const float& getSpecularity() const;
    void setSpecularity(const float specularity);

    ///
    /// \brief Add/Get texture
    ///
    void addTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(Texture::Type type);

protected:
    friend class VTKRenderDelegate;

    // State
    DisplayMode m_displayMode = DisplayMode::SURFACE;
    float m_lineWidth = 1.0;
    float m_pointSize = 1.0;
    bool m_backfaceCulling = true; ///< For performance, uncommon for this to be false

    // Colors
    Color m_diffuseColor = Color::White;
    Color m_specularColor = Color::Black;

    // Classical values
    float m_specularity = 0.0; ///< Not shiny by default

    // Textures
    std::vector<std::shared_ptr<Texture>> m_textures; ///< Ordered by Texture::Type

    bool m_stateModified = true; ///< Flag for expensive state changes
    bool m_modified = true; ///< Flag for any material property changes
};
}

#endif
