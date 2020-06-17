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

#include "imstkColor.h"
#include "imstkTextureManager.h"

namespace imstk
{
class Texture;

class RenderMaterial
{
public:
    enum class DisplayMode
    {       
        Surface,
        Wireframe,
        Points,
        WireframeSurface,
        VolumeRendering,
        Fluid
    };

    enum class ShadingModel
    {
        Phong,
        PBR, // Physicall based rendering
        Flat,
        Gouraud
    };

    enum class PointGlyphType
    {
        Sphere,
        Cube
    };

    // Volume rendering blend mode
    enum class BlendMode
    {
        Alpha,
        Additive,
        MaximumIntensity,
        MinimumIntensity
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
    /// \brief Get/Set the color. This affects the diffuse color directly, but
    /// it affects the specular color in the case of metals.
    ///
    const Color& getColor() const;
    void setColor(const Color& color);

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
    /// \brief Add/Get texture
    ///
    void addTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> getTexture(Texture::Type type);

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
    /// \brief Get/Set shadow receiving ability
    ///
    void setEdgeVisibility(const bool visibility) { m_edgeVisibility = visibility; };
    bool getEdgeVisibility() const { return m_edgeVisibility; };

    ///
    /// \brief Get/Set blend mode
    /// This function only works for volumes, particles and decals currently
    /// and the MAXIMUM_INTENSITY and MINIMUM_INTENSITY blend modes are only available for volumes
    ///
    virtual void setBlendMode(const BlendMode blendMode);
    const BlendMode getBlendMode();

    ///
    /// \brief Checks if the material must be handled uniquely
    ///
    bool isDecal();// remove?
    bool isParticle();// remove?
    bool isLineMesh();// remove?

    DisplayMode getRenderMode() const { return m_displayMode; };
    ShadingModel getShadingModel() const { return m_shadingModel; };
    void setShadingModel(const ShadingModel& model) { m_shadingModel = model; }

    bool isModified() const { return m_modified; };

    void setModified(const bool c) { m_modified=c; };

    float getOcclusionStrength() const { return m_occlusionStrength; }
    float getNormalStrength() const { return m_normalStrength; }

    const Color& getEdgeColor() const { return m_edgeColor; };
    void setEdgeColor(const Color& color) { m_edgeColor = color; };

    const Color& getVertexColor() const { return m_vertexColor; };
    void setVertexColor(const Color& color) { m_vertexColor = color; };

    double getOpacity() const { return m_opacity; }
    void setOpacity(const float opacity) { m_opacity = opacity; };

    bool getBackfaceCulling() const { return m_backfaceCulling; };
    void setBackfaceCulling(const bool c) { m_backfaceCulling = c; };

protected:
    friend class VTKRenderDelegate;
    friend class VulkanRenderDelegate;
    friend class VulkanDecalRenderDelegate;
    friend class VulkanLineMeshRenderDelegate;
    friend class VulkanParticleRenderDelegate;
    friend class VTKdbgLinesRenderDelegate;

    // State    
    bool  m_tessellated = false;//?
    
        
    bool  m_isDecal    = false;//?
    bool  m_isLineMesh = false;//?
    bool  m_isParticle = false;//?
    
    // Textures
    std::vector<std::shared_ptr<Texture>> m_textures; ///< Ordered by Texture::Type           


    ///--------------Volume rendering properties----------------    
    BlendMode m_blendMode = BlendMode::Alpha;


    ///-------------------Common properties---------------------    
    Color m_color = Color::LightGray;
    float m_opacity = 1.0;
   

    ///-------------Wireframe specific properties----------------
    PointGlyphType m_pointGlyphType = PointGlyphType::Sphere;
    float m_lineWidth = 1.f;
    float m_pointSize = 2.f;
    Color m_edgeColor = Color(0.9, 0.9, 0.4);
    Color m_vertexColor = Color(0.5, 1.0, 0.8);
    bool m_edgeVisibility = true;
    bool m_vertexVisibility = true;
    
    
    ///----------------PBR specific properties-------------------
    float m_emissivity = 0.0;
    Color m_emmisiveColor = Color::White;

    float m_metalness = 0.f; ///< Value for metalness with range: [0.0, 1.0]
    float m_roughness = 1.f; ///< Value for roughness with range: [0.0, 1.0]

    float m_occlusionStrength = 10.0;
    float m_normalStrength = 10.0;
    
    
    ///-----------------Global states/flags----------------------    
    bool m_imageBasedLighting = false;     

    // Shadows
    bool m_receivesShadows = true;//?
    bool m_castsShadows = true;//?

    // remove one of these?
    bool m_stateModified = true;        ///< Flag for expensive state changes
    bool m_modified = true;             ///< Flag for any material property changes
    bool  m_backfaceCulling = true;     ///< For performance, uncommon for this to be false

    DisplayMode m_displayMode = DisplayMode::Surface;
    ShadingModel m_shadingModel = ShadingModel::Phong;
};
}
