// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "Core/BaseMesh.h"
#include "Core/CollisionConfig.h"

namespace Core {

///
/// \brief Texture management structure
///
struct BaseMesh::TextureAttachment
{
    TextureAttachment() :
        textureId(-1),
        textureName(""),
        textureFileName("")
    {
    }

    // Texture id
    int textureId;

    // Texture internal name
    std::string textureName;

    // Texture filename
    std::string textureFileName;
};

BaseMesh::BaseMesh() : collisionGroup(nullptr), log(nullptr) {}
BaseMesh::~BaseMesh() {}
void BaseMesh::assignTexture( const int textureId )
{
    // Texture enumeration starts at 1.
    if(textureId < 1)
    {
        return;
    }

    auto attachment = std::make_shared<TextureAttachment>();
    attachment->textureId = textureId;

    this->textures.push_back(attachment);
}
void BaseMesh::assignTexture(const std::string &newTextureFileName,
                             const std::string &referenceName)
{
    auto it = std::find_if(std::begin(textures), std::end(textures),
              [referenceName](std::shared_ptr<Core::BaseMesh::TextureAttachment> &t)
                {
                    if(t->textureName != referenceName)
                        return false;
                    return true;
                });
    // If there is a texture with same name, then ignore this one.
    if(it != std::end(textures))
    {
        return;
    }
    else
    {
        auto attachment = std::make_shared<TextureAttachment>();
        attachment->textureName = referenceName;
        attachment->textureFileName = newTextureFileName;
        this->textures.push_back(attachment);
    }
}
bool BaseMesh::isMeshTextured() const
{
    return this->textures.size() > 0;
}
void BaseMesh::updateOriginalVertsWithCurrent()
{
    this->origVerts = this->vertices;
}
const std::vector<core::Vec3d>& BaseMesh::getVertices() const
{
    return this->vertices;
}
std::vector<core::Vec3d>& BaseMesh::getVertices()
{
    return this->vertices;
}
const core::Vec3d& BaseMesh::getVertex ( const size_t i ) const
{
    return this->vertices[i];
}
core::Vec3d& BaseMesh::getVertex ( const size_t i )
{
    return this->vertices[i];
}
const std::vector<core::Vec3d>& BaseMesh::getOrigVertices() const
{
    return this->origVerts;
}
std::vector<core::Vec3d>& BaseMesh::getOrigVertices()
{
    return this->origVerts;
}
std::size_t BaseMesh::getNumberOfVertices() const
{
    return this->vertices.size();
}
std::shared_ptr< CollisionGroup > &BaseMesh::getCollisionGroup()
{
    return this->collisionGroup;
}
std::size_t BaseMesh::getRenderingId() const
{
    return this->renderingID;
}
std::string BaseMesh::getTextureFileName(const size_t i) const
{
    return textures[i]->textureFileName;
}
const std::vector<core::Vec2f,Eigen::aligned_allocator<core::Vec2f>> &
BaseMesh::getTextureCoordinates() const
{
    return this->textureCoord;
}
std::vector<core::Vec2f,Eigen::aligned_allocator<core::Vec2f>> &
BaseMesh::getTextureCoordinates()
{
    return this->textureCoord;
}
const std::vector< std::shared_ptr< BaseMesh::TextureAttachment > > &
BaseMesh::getTextures() const
{
    return this->textures;
}
const int& BaseMesh::getTextureId(size_t i) const
{
    return this->textures[i]->textureId;
}
bool BaseMesh::BaseMesh::hasTextureCoordinates() const
{
    return this->textureCoord.size() > 0;
}
void BaseMesh::addTextureCoordinate ( const core::Vec2f& coord )
{
    this->textureCoord.push_back( coord );
}
void BaseMesh::addTextureCoordinate ( const float& x, const float& y )
{
    this->textureCoord.push_back( core::Vec2f(x,y) );
}
void BaseMesh::setBoundingBox ( const Eigen::AlignedBox3d& box )
{
    this->aabb = box;
}
const Eigen::AlignedBox3d& BaseMesh::getBoundingBox() const
{
    return this->aabb;
}
void BaseMesh::setRenderingId( size_t id )
{
    this->renderingID = id;
}
void BaseMesh::translate ( const Eigen::Translation3d& translation )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [translation] ( core::Vec3d &v )
    {
        v = translation*v;
    } );
}
void BaseMesh::scale ( const Eigen::UniformScaling<double>& scaling )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [scaling] ( core::Vec3d &v )
    {
        v = scaling*v;
    } );
}
void BaseMesh::rotate ( const Quaterniond& rotation )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [rotation] ( core::Vec3d &v )
    {
        v = rotation*v;
    } );
}
void BaseMesh::transform ( const Eigen::Transform<double,3,Eigen::Affine>& transformation )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [transformation] ( core::Vec3d &v )
    {
        v = transformation*v;
    } );
}


}
