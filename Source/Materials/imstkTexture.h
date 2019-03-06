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

#ifndef imstkTexture_h
#define imstkTexture_h

#include <string>
#include <memory>

#include "g3log/g3log.hpp"

namespace imstk
{
///
/// \class Texture
///
class Texture
{
public:
    ///
    /// \brief Texture type - determines filtering
    ///
    enum class Type
    {
        DIFFUSE = 0, // Also used for albedo
        NORMAL,
        ROUGHNESS,
        METALNESS,
        SUBSURFACE_SCATTERING,
        AMBIENT_OCCLUSION,
        CAVITY,
        CUBEMAP,
        IRRADIANCE_CUBEMAP,
        RADIANCE_CUBEMAP,
        BRDF_LUT,
        NONE
    };

    enum class FileType
    {
        UNKNOWN,
        BMP,
        PNG,
        JPG,
        DDS
    };

    ///
    /// \brief Constructor
    /// \param path Path to the texture source file
    /// \param type Type of texture
    ///
    Texture(std::string path = "", Type type = Type::DIFFUSE);

    ///
    /// \brief Destructor
    ///
    virtual ~Texture() {}

    ///
    /// \brief Get type
    ///
    Type getType() const;

    ///
    /// \brief Get type as a string
    ///
    std::string getTypeAsString() const;

    ///
    /// \brief Convert a Type into a string
    ///
    static std::string getTypeAsString(Type type);

    ///
    /// \brief Get path
    ///
    const std::string getPath() const;

    ///
    /// \brief Get file extension
    ///
    const FileType getFileType();

    ///
    /// \brief Get if mipmaps are enabled
    ///
    bool getMipmapsEnabled();

    ///
    /// \brief Get if anisotropic filtering is enabled
    ///
    bool isAnisotropyEnabled();

    ///
    /// brief Get anisotropy factor
    ///
    float getAnisotropyFactor();

protected:
    Type m_type;            ///< Texture type
    std::string m_path;     ///< Texture file path

    // Helps with texture aliasing (and a little with performance)
    bool m_mipmapsEnabled = true;

    // Helps sharpen mipmapped textures at more extreme angles
    bool m_anisotropyEnabled = true;
    float m_anisotropyFactor = 1.0;
};
}

// This method is defined to allow for the map to be properly indexed by Texture objects
namespace std
{
template<> struct less<std::shared_ptr<imstk::Texture>>
{
    bool operator() (const std::shared_ptr<imstk::Texture>& texture1,
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
}

#endif
