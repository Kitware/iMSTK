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

namespace Core {

BaseMesh::BaseMesh() {}
BaseMesh::~BaseMesh() {}
void BaseMesh::updateInitialVertices()
{
    this->origVerts = this->vertices;
}
void BaseMesh::resetVertices()
{
    this->vertices = this->origVerts;
}
const std::vector<core::Vec3d>& BaseMesh::getVertices() const
{
    return this->vertices;
}
std::vector<core::Vec3d>& BaseMesh::getVertices()
{
    return this->vertices;
}
const std::vector< std::array<std::size_t,3>>& BaseMesh::getTriangles() const
{
    return this->triangleArray;
}
std::vector< std::array<std::size_t,3>>& BaseMesh::getTriangles()
{
    return this->triangleArray;
}
void BaseMesh::setTriangles(const std::vector< std::array<std::size_t,3>>& triangles)
{
    this->triangleArray = triangles;
}
const std::vector< std::array<std::size_t,4>>& BaseMesh::getTetrahedrons() const
{
    return this->tetrahedraArray;
}
std::vector< std::array<std::size_t,4>>& BaseMesh::getTetrahedrons()
{
    return this->tetrahedraArray;
}
void BaseMesh::setTetrahedrons(const std::vector<std::array<std::size_t,4>>& tetrahedrons)
{
    this->tetrahedraArray = tetrahedrons;
}
const std::vector< std::array<std::size_t,8> >& BaseMesh::getHexahedrons() const
{
    return this->hexahedraArray;
}
std::vector< std::array<std::size_t,8>>& BaseMesh::getHexahedrons()
{
    return this->hexahedraArray;
}
void BaseMesh::setHexahedrons(const std::vector<std::array< std::size_t,8>>& hexahedrons)
{
    this->hexahedraArray = hexahedrons;
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
std::size_t BaseMesh::getRenderingId() const
{
    return this->renderingID;
}
void BaseMesh::setRenderingId( size_t id )
{
    this->renderingID = id;
}
void BaseMesh::translate( const Eigen::Translation3d& translation, bool setInitialPoints )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [translation] ( core::Vec3d &v )
    {
        v = translation*v;
    } );
    if(setInitialPoints)
    {
        this->updateInitialVertices();
    }
}
void BaseMesh::scale ( const Eigen::UniformScaling<double>& scaling )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [scaling] ( core::Vec3d &v )
    {
        v = scaling*v;
    } );
}
void BaseMesh::rotate ( const core::Quaterniond& rotation )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [rotation] ( core::Vec3d &v )
    {
        v = rotation*v;
    } );
}
void BaseMesh::transform ( const TransformType& transformation )
{
    std::for_each ( std::begin(vertices),std::end(vertices),
                    [transformation] ( core::Vec3d &v )
    {
        v = transformation*v;
    } );
}
void BaseMesh::setVertices(const std::vector<core::Vec3d>& vertices)
{
    this->vertices = vertices;
}
std::array<core::Vec3d,3> BaseMesh::getTriangleVertices(size_t i) const
{
    std::array<core::Vec3d,3> triangleVertices =
    {
        this->vertices[this->triangleArray[i][0]],
        this->vertices[this->triangleArray[i][1]],
        this->vertices[this->triangleArray[i][2]]
    };
    return std::move(triangleVertices);
}
int BaseMesh::getMeshType()
{
  return this->meshType;
}
void BaseMesh::setMeshType(int meshType)
{
  this->meshType = meshType;
}

}
