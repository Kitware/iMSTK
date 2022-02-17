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
///
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
        Image,
        SurfaceNormals
    };

    /// surface shading model. Defaults to Phong
    enum class ShadingModel
    {
        None,    ///< Renders without shading, no lighting
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

    RenderMaterial();
    virtual ~RenderMaterial() = default;

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

    ///
    /// \brief Get/Set the material name
    ///@{
    const std::string& getName() const { return m_name; }
    void setName(std::string name) { m_name = name; }
    ///@}

    ///
    /// \brief Get/Set display mode
    ///@{
    DisplayMode getDisplayMode() const { return m_displayMode; }
    void setDisplayMode(const DisplayMode displayMode);
    ///@}

    ///
    /// \brief Get/Set line width or the wireframe
    ///@{
    double getLineWidth() const { return m_lineWidth; }
    void setLineWidth(const double width);
    ///@}

    ///
    /// \brief Get/Set point radius
    ///@{
    double getPointSize() const { return m_pointSize; }
    void setPointSize(const double size);
    ///@}

    ///
    /// \brief Backface culling on/off
    ///@{
    bool getBackFaceCulling() const { return m_backfaceCulling; }
    void setBackFaceCulling(const bool culling);
    void backfaceCullingOn();
    void backfaceCullingOff();
    ///@}

    ///
    /// \brief Get/Set the color. This affects the diffuse color directly, but
    /// it affects the specular color in the case of metals.
    ///@{
    const Color& getDiffuseColor() const { return m_diffuseColor; }
    void setDiffuseColor(const Color& color);
    const Color& getColor() const { return this->getDiffuseColor(); }
    void setColor(const Color& color);
    ///@}

    ///
    /// \brief Get/Set the specular color
    ///@{
    const Color& getSpecularColor() const { return m_specularColor; }
    void setSpecularColor(const Color& color);
    ///@}

    ///
    /// \brief Get/Set the specular amount
    ///@{
    double getSpecular() const { return m_specular; }
    void setSpecular(const double specular) { m_specular = specular; }
    ///@}

    ///
    /// \brief Get/Set the ambient color
    ///@{
    const Color& getAmbientColor() const { return m_ambientColor; }
    void setAmbientColor(const Color& color);
    ///@}

    ///
    /// \brief Get/Set the metalness
    ///@{
    double getMetalness() const { return m_metalness; }
    void setMetalness(const double metalness);
    ///@}

    ///
    /// \brief Get/Set ambient light coefficient
    ///@{
    double getAmbient() const { return m_ambient; };
    void setAmbient(const double ambient) { m_ambient = ambient; }
    ///@}

    ///
    /// \brief Get/Set diffuse light coefficient
    ///@{
    double getDiffuse() const { return m_diffuse; }
    void setDiffuse(const double diffuse) { m_diffuse = diffuse; }
    ///@}

    ///
    /// \brief Get/Set ambient light coefficient
    ///@{
    double getSpecularPower() const { return m_specularPower; }
    void setSpecularPower(const double p) { m_specularPower = p; }
    ///@}

    ///
    /// \brief Get/Set the roughness
    ///@{
    double getRoughness() const { return m_roughness; }
    void setRoughness(const double roughness);
    ///@}

    ///
    /// \brief Get/Set emissivity
    ///@{
    double getEmissivity() const { return m_emissivity; }
    void setEmissivity(const double emissivity);
    ///@}

    double getAnisotropy() const { return m_anisotropy; }
    void setAnisotropy(const double anisotropy);

    double getAnisotropyRotation() const { return m_anisotropyRotation; }
    void setAnisotropyRotation(const double anisotropyRotation);

    double getBaseIOR() const { return m_baseIOR; }
    void setBaseIOR(const double baseIOR);

    const Color& getCoatColor() const { return m_coatColor; }
    void setCoatColor(const Color& coatColor);

    double getCoatIOR() const { return m_coatIOR; }
    void setCoatIOR(const double coatIOR);

    double getCoatNormalScale() const { return m_coatNormalScale; }
    void setCoatNormalScale(const double coatNormalScale);

    double getCoatRoughness() const { return m_coatRoughness; }
    void setCoatRoughness(const double coatRoughness);

    double getCoatStrength() const { return m_coatStrength; }
    void setCoatStrength(const double coatStrength);

    const Color& getEdgeTint() const { return m_edgeTint; }
    void setEdgeTint(const Color& edgeTint);

    ///
    /// \brief Add/Remove/Get texture
    ///@{
    void addTexture(std::shared_ptr<Texture> texture);
    void removeTexture(std::shared_ptr<Texture> texture);
    void removeTexture(Texture::Type type);
    std::shared_ptr<Texture> getTexture(Texture::Type type);
    ///@}

    ///
    /// \brief Get/Set edge visibility
    ///@{
    void setEdgeVisibility(const bool visibility) { m_edgeVisibility = visibility; };
    bool getEdgeVisibility() const { return m_edgeVisibility; };
    ///@}

    ///
    /// \brief Get/Set blend mode
    /// This function only works for volumes, particles and decals currently
    /// and the MAXIMUM_INTENSITY and MINIMUM_INTENSITY blend modes are only available for volumes
    ///@{
    virtual void setBlendMode(const BlendMode blendMode);
    BlendMode getBlendMode() const { return m_blendMode; }
    ///@}

    DisplayMode getRenderMode() const { return m_displayMode; }
    ShadingModel getShadingModel() const { return m_shadingModel; }
    void setShadingModel(const ShadingModel& model);

    double getOcclusionStrength() const { return m_occlusionStrength; }
    void setOcclusionStrength(const double o);

    double getNormalStrength() const { return m_normalStrength; }
    void setNormalStrength(const double n);

    const Color& getEdgeColor() const { return m_edgeColor; }
    void setEdgeColor(const Color& color);

    const Color& getVertexColor() const { return m_vertexColor; }
    void setVertexColor(const Color& color);

    double getOpacity() const { return m_opacity; }
    void setOpacity(const double opacity);

    bool getBackfaceCulling() const { return m_backfaceCulling; }
    void setBackfaceCulling(const bool c);

    std::shared_ptr<ColorFunction> getColorLookupTable() const { return m_lookupTable; }
    void setColorLookupTable(std::shared_ptr<ColorFunction> lut);

    bool getScalarVisibility() const { return m_scalarVisibility; }
    void setScalarVisibility(const bool scalarVisibility);

    bool getRecomputeVertexNormals() const { return m_recomputeVertexNormals; }
    void setRecomputeVertexNormals(const bool recomputeVertexNormals) { m_recomputeVertexNormals = recomputeVertexNormals; }

    bool getRenderPointsAsSpheres() const { return m_renderPointsAsSpheres; }
    void setRenderPointsAsSpheres(const bool renderPointsAsSpheres) { m_renderPointsAsSpheres = renderPointsAsSpheres; }

    bool getIsDynamicMesh() const { return m_isDynamicMesh; }
    void setIsDynamicMesh(const bool isDynamicMesh) { m_isDynamicMesh = isDynamicMesh; }

    void postModified() { this->postEvent(Event(RenderMaterial::modified())); }

protected:
    std::string m_name = "";

    // Textures
    std::vector<std::shared_ptr<Texture>> m_textures; ///< Ordered by Texture::Type

    ///--------------Volume rendering properties----------------
    BlendMode m_blendMode = BlendMode::Alpha;

    ///-------------------Common properties---------------------
    double m_diffuse      = 1.0;
    Color  m_diffuseColor = Color::LightGray;

    double m_specularPower = 100.0;
    double m_specular      = 0.0;
    Color  m_specularColor = Color::Red;

    double m_ambient      = 0.1;
    Color  m_ambientColor = Color::White;

    double m_opacity = 1.0;

    ///-------------Wireframe specific properties----------------
    double m_lineWidth        = 1.0;
    double m_pointSize        = 2.0;
    Color  m_edgeColor        = Color::Marigold;
    Color  m_vertexColor      = Color::Teal;
    bool   m_edgeVisibility   = true; ///< \note not used (vtk backend)
    bool   m_vertexVisibility = true; ///< \note not used (vtk backend)

    ///----------------PBR specific properties-------------------
    double m_emissivity    = 0.0;
    Color  m_emmisiveColor = Color::White;

    double m_metalness = 1.0; ///< Value for metalness with range: [0.0, 1.0]
    double m_roughness = 1.0; ///< Value for roughness with range: [0.0, 1.0]
    double m_occlusionStrength = 1.0;
    double m_normalStrength    = 1.0;

    ///----------------PBR Clearcoat properties-------------------
    double m_anisotropy = 0.0;
    double m_anisotropyRotation = 0.0;
    double m_baseIOR         = 1.5; // Base index of refraction
    Color  m_coatColor       = Color::White;
    double m_coatIOR         = 2.0; // Coat index of refraction
    double m_coatNormalScale = 1.0;
    double m_coatRoughness   = 0.0;
    double m_coatStrength    = 0.0;
    Color  m_edgeTint        = Color::White;

    bool m_backfaceCulling = true; ///< For performance, uncommon for this to be false
    bool m_isDynamicMesh   = true; ///< Dynamic mesh is one whose buffers are re-evaluated on update

    DisplayMode  m_displayMode  = DisplayMode::Surface;
    ShadingModel m_shadingModel = ShadingModel::Phong;

    bool m_recomputeVertexNormals = true; ///> Update vertex normals when vertex or index data is changed

    std::shared_ptr<ColorFunction> m_lookupTable;
    bool m_scalarVisibility = false;

    bool m_renderPointsAsSpheres = false;
};
} // namespace imstk
