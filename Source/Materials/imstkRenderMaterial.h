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
    DisplayMode getDisplayMode() const;
    void setDisplayMode(const DisplayMode displayMode);

    ///
    /// \brief Get/Set tessellated
    ///
    bool getTessellated() const;
    void setTessellated(const bool tessellated);

    ///
    /// \brief Get/Set line width or the wireframe
    ///
    float getLineWidth() const;
    void setLineWidth(const float width);

    ///
    /// \brief Get/Set point size
    ///
    float getPointSize() const;
    void setPointSize(const float size);

    ///
    /// \brief Backface culling on/off
    ///
    bool getBackFaceCulling() const;
    void setBackFaceCulling(const bool culling);
    void backfaceCullingOn();
    void backfaceCullingOff();

    ///
    /// \brief Get/Set the diffuse color
    ///
    const Color& getColor() const;
    void setColor(const Color color);

    ///
    /// \brief Get/Set the color of the wireframe and points
    ///
    const Color& getDebugColor() const;
    void setDebugColor(const Color color);

    ///
    /// \brief Get/Set the metalness
    ///
    const float& getMetalness() const;
    void setMetalness(const float metalness);

    ///
    /// \brief Get/Set the roughness
    ///
    const float& getRoughness() const;
    void setRoughness(const float roughness);

    ///
    /// \brief Get/Set emissivity
    ///
    const float& getEmissivity() const;
    void setEmissivity(const float emissivity);

    ///
    /// \brief Get/Set the roughness
    ///
    const double& getSphereGlyphSize() const;
    void setSphereGlyphSize(const double size);

    ///
    /// \brief Add/Get texture
    ///
    void addTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(Texture::Type type);

    ///
    /// \brief Turn on/off flat shading
    ///
    void flatShadingOn();
    void flatShadingOff();

    ///
    /// \brief Turn on/off visibility
    ///
    void setVisibilityOff();
    void setVisibilityOn();
    bool getVisibility() const;

    ///
    /// \brief Get/Set shadow receiving ability
    ///
    void setReceivesShadows(const bool receivesShadows);
    bool getReceivesShadows() const;

    ///
    /// \brief Get/Set shadow cast status
    ///
    void setCastsShadows(const bool castsShadows);
    bool getCastsShadows() const;

    ///
    /// \brief Checks if the material belongs to a decal
    ///
    bool isDecal();

    bool isLineMesh();

protected:
    friend class VTKRenderDelegate;
    friend class VulkanRenderDelegate;
    friend class VulkanDecalRenderDelegate;
    friend class VulkanLineMeshRenderDelegate;

    // State
    DisplayMode m_displayMode = DisplayMode::SURFACE;
    bool m_tessellated = false;
    float m_lineWidth = 1.0;
    float m_pointSize = 1.0;
    bool m_backfaceCulling = true; ///< For performance, uncommon for this to be false
    bool m_isDecal = false;
    bool m_isLineMesh = false;

    // Sphere size used for glyph in rendering (valid only for point set)
    double m_sphereGlyphSize = 0.05;

    // Colors
    Color m_color = Color::White;
    Color m_debugColor = Color::Black;

    // Classical values
    float m_metalness = 0.0; ///< Value for metalness with range: [0.0, 1.0]
    float m_roughness = 1.0; ///< Value for roughness with range: [0.0, 1.0]
    float m_emissivity = 0.0;

    // Textures
    std::vector<std::shared_ptr<Texture>> m_textures; ///< Ordered by Texture::Type

    // Shadows
    bool m_receivesShadows = true;
    bool m_castsShadows = true;

    // Visibility
    bool m_isVisible = true;

    bool m_stateModified = true; ///< Flag for expensive state changes
    bool m_modified = true;      ///< Flag for any material property changes
    bool m_flatShading = false;
};
}

#endif
