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

#include "imstkEventObject.h"
#include "imstkColor.h"

#include <string>
#include <memory>

namespace imstk
{
class ImageData;

///
/// \class Texture
///
/// \brief A texture can be defined by file reference or ImageData input
///
class Texture : public EventObject
{
public:
    ///
    /// \brief Texture type - determines filtering
    ///
    enum class Type
    {
        Diffuse = 0, // Also used for albedo
        Normal,
        Roughness,
        Metalness,
        SubsurfaceScattering,
        AmbientOcclusion,
        Cavity,
        Cubemap,
        IrradianceCubeMap,
        RadianceCubeMap,
        ORM,
        BRDF_LUT,
        Emissive,
        Anistropy,
        CoatNormal,
        None
    };

    ///
    /// \brief TODO
    ///
    enum class FileType
    {
        Unknown,
        Bmp,
        Png,
        Jpg,
        Dds
    };

    enum class WrapType
    {
        CLAMP_TO_EDGE,   // Clamps without border color
        CLAMP_TO_BORDER, // Pixels outside [0,1] use border color
        REPEAT           // Pixels outside [0,1] repeat back to [0,1] in a modulus fashion. Such that 1.3, becomes 0.3
    };

    ///
    /// \brief Constructor
    /// \param path Path to the texture source file
    /// \param type Type of texture
    ///
    Texture(std::string path = "", Type type = Type::Diffuse);

    ///
    /// \brief Constructor
    /// \param imageTex input texture image
    /// \param type Type of texture
    ///
    Texture(std::shared_ptr<ImageData> imageTex, Type type = Type::Diffuse);

    ///
    /// \brief Destructor
    ///
    virtual ~Texture() = default;

    // *INDENT-OFF*
    SIGNAL(Texture, modified);
    // *INDENT-ON*

    void postModified() { this->postEvent(Event(modified())); }

    ///
    /// \brief Get type
    ///
    Type getType() const;

    ///
    /// \brief Get path
    ///
    const std::string& getPath() const { return m_path; }

    ///
    /// \brief Get file extension
    ///
    FileType getFileType();

    ///
    /// \brief Get if mipmaps are enabled
    ///
    bool getMipmapsEnabled() const { return m_mipmapsEnabled; }

    ///
    /// \brief Get the wrapping type
    ///
    const WrapType& getWrapType() const { return m_wrapType; }
    void setWrapType(const WrapType& repeat)
    {
        m_wrapType = repeat;
        postModified();
    }

    const Color& getBorderColor() const { return m_borderColor; }
    void setBorderColor(const Color& color)
    {
        m_borderColor = color;
        postModified();
    }

    ///
    /// \brief Get if anisotropic filtering is enabled
    ///
    bool isAnisotropyEnabled() const { return m_anisotropyEnabled; }

    ///
    /// \brief Get anisotropy factor
    ///
    double getAnisotropyFactor() const { return m_anisotropyFactor; }

    ///
    /// \brief Set the input image data, not required (paths to files can be used instead)
    ///
    void setImageData(std::shared_ptr<ImageData> imgData) { imageTexture = imgData; }

    ///
    /// \brief Set whether interpolation is used when sampling the texture
    ///
    void setInterpolation(const bool interpolation)
    {
        m_interpolation = interpolation;
        postModified();
    }

    ///
    /// \brief Get whether interpolation is used when sampling the texture
    ///
    bool getInterpolation() { return m_interpolation; }

    ///
    /// \brief Get the input image data for the texture, not required (paths to files can be used instead)
    ///
    std::shared_ptr<ImageData> getImageData() const { return imageTexture; }

protected:
    std::shared_ptr<ImageData> imageTexture = nullptr;
    Type m_type;             ///< Texture type
    std::string m_path = ""; ///< Texture file path

    // Helps with texture aliasing (and a little with performance)
    bool m_mipmapsEnabled = true;

    WrapType m_wrapType    = WrapType::REPEAT;
    Color    m_borderColor = Color::Black;

    // Helps sharpen mipmapped textures at more extreme angles
    bool   m_anisotropyEnabled = true;
    double m_anisotropyFactor  = 1.0;

    // Use interpolation when texturing?
    bool m_interpolation = true;
};
} // namespace imstk

namespace std
{
///
/// \struct less
/// \brief This method is defined to allow for the map to be properly indexed by Texture objects
///
template<> struct less<std::shared_ptr<imstk::Texture>>
{
    bool operator()(const std::shared_ptr<imstk::Texture>& texture1,
                    const std::shared_ptr<imstk::Texture>& texture2) const
    {
        if (texture1->getType() != texture2->getType())
        {
            return (texture1->getType() < texture2->getType());
        }

        if (texture1->getPath() != texture2->getPath())
        {
            return (texture1->getPath() < texture2->getPath());
        }

        if (texture1->getMipmapsEnabled() != texture2->getMipmapsEnabled())
        {
            return (texture1->getMipmapsEnabled() < texture2->getMipmapsEnabled());
        }

        if (texture1->isAnisotropyEnabled() != texture2->isAnisotropyEnabled())
        {
            return (texture1->isAnisotropyEnabled() < texture2->isAnisotropyEnabled());
        }

        if (texture1->getAnisotropyFactor() != texture2->getAnisotropyFactor())
        {
            return (texture1->getAnisotropyFactor() < texture2->getAnisotropyFactor());
        }

        return false;
    }
};
} // namespace std
