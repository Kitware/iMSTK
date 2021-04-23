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
#include "imstkEventObject.h"
#include "imstkTexture.h"

#include <vector>

namespace imstk
{
class ColorFunction;
class Texture;

///
/// \class RenderMaterial
/// \brief Serves no function, just a database for render properties, emits events for
/// others to observe its changes
///
class RenderMaterial : public EventObject
{
public:
    /// Display mode for the scene objects
    enum class DisplayMode
    {
        Surface,
        Wireframe,
        Points,
        WireframeSurface,
        VolumeRendering,
        Fluid,               ///< Renders a set of points using a screen-space fluid renderer
        Image
    };

    /// surface shading model. Defaults to Phong
    enum class ShadingModel
    {
        Phong,   ///< Phong shading model (default)
        Gouraud, ///< Gouraud shading model (default)
        Flat,    ///< Flat shading model with no interpolation
        PBR      ///< Physically based rendering
    };

    /// Volume rendering blend mode
    enum class BlendMode
    {
        Alpha,
        Additive,
        MaximumIntensity,
        MinimumIntensity
    };

public:
    RenderMaterial();
    virtual ~RenderMaterial() = default;

public:
    // *INDENT-OFF*
    ///
    /// \brief Posted when material parameters are modified
    ///
    SIGNAL(RenderMaterial, modified);

    ///
    /// \brief Posted when textures are modified
    ///
    SIGNAL(RenderMaterial, texturesModified);

    // *INDENT-ON*

public:
    ///
    /// \brief Get/Set the material name
    ///
    const std::string& getName() const { return m_name; }
    void setName(std::string name) { m_name = name; }

    ///
    /// \brief Get/Set display mode
    ///
    const DisplayMode getDisplayMode() const { return m_displayMode; }
    void setDisplayMode(const DisplayMode displayMode);

    ///
    /// \brief Get/Set tessellated
    ///
    const bool getTessellated() const { return m_tessellated; }
    void setTessellated(const bool tessellated);

    ///
    /// \brief Get/Set line width or the wireframe
    ///
    const float getLineWidth() const { return m_lineWidth; }
    void setLineWidth(const float width);

    ///
    /// \brief Get/Set point radius
    ///
    const float getPointSize() const { return m_pointSize; }
    void setPointSize(const float size);

    ///
    /// \brief Backface culling on/off
    ///
    const bool getBackFaceCulling() const { return m_backfaceCulling; }
    void setBackFaceCulling(const bool culling);
    void backfaceCullingOn();
    void backfaceCullingOff();

    ///
    /// \brief Get/Set the color. This affects the diffuse color directly, but
    /// it affects the specular color in the case of metals.
    ///
    const Color& getDiffuseColor() const { return m_diffuseColor; }
    void setDiffuseColor(const Color& color);
    const Color& getColor() const { return this->getDiffuseColor(); }
    void setColor(const Color& color);

    ///
    /// \brief Get/Set the specular color
    ///
    const Color& getSpecularColor() const { return m_specularColor; }
    void setSpecularColor(const Color& color);

    ///
    /// \brief Get/Set the specular amount
    ///
    float getSpecular() const { return m_specular; }
    void setSpecular(float specular) { m_specular = specular; }

    ///
    /// \brief Get/Set the ambient color
    ///
    const Color& getAmbientColor() const { return m_ambientColor; }
    void setAmbientColor(const Color& color);

    ///
    /// \brief Get/Set the metalness
    ///
    const float getMetalness() const { return m_metalness; }
    void setMetalness(const float metalness);

    ///
    /// \brief Get/Set ambient light coefficient
    ///
    const float getAmbient() const { return m_ambient; };
    void setAmbient(float ambient) { m_ambient = ambient; }

    ///
    /// \brief Get/Set diffuse light coefficient
    ///
    const float getDiffuse() const { return m_diffuse; }
    void setDiffuse(float diffuse) { m_diffuse = diffuse; }

    ///
    /// \brief Get/Set ambient light coefficient
    ///
    const float getSpecularPower() const { return m_specularPower; }
    void setSpecularPower(const float p) { m_specularPower = p; }

    ///
    /// \brief Get/Set the roughness
    ///
    const float getRoughness() const { return m_roughness; }
    void setRoughness(const float roughness);

    ///
    /// \brief Get/Set emissivity
    ///
    const float getEmissivity() const { return m_emissivity; }
    void setEmissivity(const float emissivity);

    ///
    /// \brief Add/Remove/Get texture
    ///
    void addTexture(std::shared_ptr<Texture> texture);
    void removeTexture(std::shared_ptr<Texture> texture);
    void removeTexture(Texture::Type type);
    std::shared_ptr<Texture> getTexture(Texture::Type type);

    ///
    /// \brief Get/Set shadow receiving ability
    ///
    void setReceivesShadows(const bool receivesShadows);
    bool getReceivesShadows() const { return m_receivesShadows; }

    ///
    /// \brief Get/Set shadow cast status
    ///
    void setCastsShadows(const bool castsShadows);
    bool getCastsShadows() const { return m_castsShadows; }

    ///
    /// \brief Get/Set edge visibility
    ///
    void setEdgeVisibility(const bool visibility) { m_edgeVisibility = visibility; };
    bool getEdgeVisibility() const { return m_edgeVisibility; };

    ///
    /// \brief Get/Set blend mode
    /// This function only works for volumes, particles and decals currently
    /// and the MAXIMUM_INTENSITY and MINIMUM_INTENSITY blend modes are only available for volumes
    ///
    virtual void setBlendMode(const BlendMode blendMode);
    const BlendMode getBlendMode() const { return m_blendMode; }

    ///
    /// \brief Checks if the material must be handled uniquely
    ///
    bool isDecal() const { return m_isDecal; }
    bool isParticle() const { return m_isParticle; }
    bool isLineMesh() const { return m_isLineMesh; }

    const DisplayMode getRenderMode() const { return m_displayMode; }
    const ShadingModel getShadingModel() const { return m_shadingModel; }
    void setShadingModel(const ShadingModel& model);

    float getOcclusionStrength() const { return m_occlusionStrength; }
    void setOcclusionStrength(const float o);

    float getNormalStrength() const { return m_normalStrength; }
    void setNormalStrength(const float n);

    float getIndexOfRefraction() const { return m_indexOfRefraction; }
    void setIndexOfRefraction(const float n);

    const Color& getEdgeColor() const { return m_edgeColor; }
    void setEdgeColor(const Color& color);

    const Color& getVertexColor() const { return m_vertexColor; }
    void setVertexColor(const Color& color);

    const double getOpacity() const { return m_opacity; }
    void setOpacity(const float opacity);

    const bool getBackfaceCulling() const { return m_backfaceCulling; }
    void setBackfaceCulling(const bool c);

    std::shared_ptr<ColorFunction> getColorLookupTable() const { return m_lookupTable; }
    void setColorLookupTable(std::shared_ptr<ColorFunction> lut);

    const bool getScalarVisibility() const { return m_scalarVisibility; }
    void setScalarVisibility(const bool scalarVisibility);

    const bool getRecomputeVertexNormals() const { return m_recomputeVertexNormals; }
    void setRecomputeVertexNormals(const bool recomputeVertexNormals) { m_recomputeVertexNormals = recomputeVertexNormals; }

    const bool getRenderPointsAsSpheres() const { return m_renderPointsAsSpheres; }
    void setRenderPointsAsSpheres(const bool renderPointsAsSpheres) { m_renderPointsAsSpheres = renderPointsAsSpheres; }

    void postModified() { this->postEvent(Event(RenderMaterial::modified())); }

protected:
    friend class VTKRenderDelegate;
    friend class VulkanRenderDelegate;
    friend class VulkanDecalRenderDelegate;
    friend class VulkanLineMeshRenderDelegate;
    friend class VulkanParticleRenderDelegate;
    friend class VTKdbgLinesRenderDelegate;

    std::string m_name = "";

    // Textures
    std::vector<std::shared_ptr<Texture>> m_textures; ///< Ordered by Texture::Type

    ///--------------Volume rendering properties----------------
    BlendMode m_blendMode = BlendMode::Alpha;

    ///-------------------Common properties---------------------
    float m_diffuse      = 1.0f;
    Color m_diffuseColor = Color::LightGray;

    float m_specularPower = 100.0f;
    float m_specular      = 0.0f;
    Color m_specularColor = Color::Red;

    float m_ambient      = 0.1f;
    Color m_ambientColor = Color::White;

    float m_opacity = 1.0f;

    ///-------------Wireframe specific properties----------------
    float m_lineWidth        = 1.f;
    float m_pointSize        = 2.f;
    Color m_edgeColor        = Color::Marigold;
    Color m_vertexColor      = Color::Teal;
    bool  m_edgeVisibility   = true; ///< \note not used (vtk backend)
    bool  m_vertexVisibility = true; ///< \note not used (vtk backend)

    ///----------------PBR specific properties-------------------
    float m_emissivity    = 0.f;
    Color m_emmisiveColor = Color::White;

    float m_metalness = 1.0f; ///< Value for metalness with range: [0.0, 1.0]
    float m_roughness = 1.0f; ///< Value for roughness with range: [0.0, 1.0]
    float m_occlusionStrength = 1.0f;
    float m_normalStrength    = 1.0f;
    float m_indexOfRefraction = 1.0f;

    ///---------------------Global states------------------------
    bool m_imageBasedLighting = false;

    // Shadows
    bool m_receivesShadows = true; ///< \note not implemented
    bool m_castsShadows    = true; ///< \note not implemented

    bool m_backfaceCulling = true; ///< For performance, uncommon for this to be false

    DisplayMode  m_displayMode  = DisplayMode::Surface;
    ShadingModel m_shadingModel = ShadingModel::Phong;

    bool m_tessellated = false;
    bool m_isDecal     = false;
    bool m_isLineMesh  = false;
    bool m_isParticle  = false;
    bool m_recomputeVertexNormals = true; ///> Update vertex normals when vertex or index data is changed

    std::shared_ptr<ColorFunction> m_lookupTable;
    bool m_scalarVisibility = false;

    bool m_renderPointsAsSpheres = false;
};
}
