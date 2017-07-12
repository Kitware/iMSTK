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

namespace imstk
{
///
/// \class Texture
///
/// \brief iMSTK texture class. There are a few texture types that
///     dictate how texture are to be treated.
///
class Texture
{
public:
    ///
    /// \brief Texture type - determines filtering
    ///
    enum Type
    {
        DIFFUSE = 0, // Also used for albedo
        NORMAL,
        SPECULAR,
        ROUGHNESS,
        METALNESS,
        AMBIENT_OCCLUSION,
        CAVITY,
        CUBEMAP,
        NONE
    };

    ///
    /// \brief Constructor
    /// \param path Path to the texture source file
    /// \param type Type of texture
    ///
    Texture(std::string path = "", Type type = DIFFUSE);

    ///
    /// \brief Destructor
    ///
    virtual ~Texture() {}

    ///
    /// \brief Get type
    ///
    Type getType() const;

    ///
    /// \brief Get path
    ///
    const std::string getPath() const;

protected:

    Type m_type;            ///< Texture type
    std::string m_path;     ///< Texture file path
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
        if (texture2->getType() != texture2->getType())
        {
            return (texture2->getType() < texture2->getType());
        }

        if (texture1->getPath() != texture2->getPath())
        {
            return (texture1->getPath() < texture2->getPath());
        }

        return false;
    }
};
}

#endif
