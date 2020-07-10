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

#include "imstkRenderParticles.h"
#include "imstkLogger.h"

namespace imstk
{
RenderParticles::RenderParticles(const unsigned int maxNumParticles /*=128*/, const std::string& name /*= std::string("")*/)
    : Geometry(Geometry::Type::RenderParticles, name)
{
    if (maxNumParticles <= 128)
    {
        m_maxNumParticles = maxNumParticles;
    }
    else
    {
        m_maxNumParticles = 128;
        LOG(WARNING) << "The maximum number of decals is 128";
    }

    m_vertexPositions[0] = glm::vec3(0.5, 0.5, 0);
    m_vertexPositions[1] = glm::vec3(0.5, -0.5, 0);
    m_vertexPositions[2] = glm::vec3(-0.5, 0.5, 0);
    m_vertexPositions[3] = glm::vec3(-0.5, -0.5, 0);

    m_vertexNormals[0] = glm::vec3(0.0, 0.0, 1.0);
    m_vertexNormals[1] = glm::vec3(0.0, 0.0, 1.0);
    m_vertexNormals[2] = glm::vec3(0.0, 0.0, 1.0);
    m_vertexNormals[3] = glm::vec3(0.0, 0.0, 1.0);

    m_vertexUVs[0] = glm::vec2(1.0, 1.0);
    m_vertexUVs[1] = glm::vec2(1.0, 0);
    m_vertexUVs[2] = glm::vec2(0, 1.0);
    m_vertexUVs[3] = glm::vec2(0, 0);

    m_triangles[0] = glm::ivec3(1, 0, 3);
    m_triangles[1] = glm::ivec3(0, 2, 3);
}

void
RenderParticles::setParticleSize(const float size)
{
    m_particleSize = size;
}

std::vector<std::unique_ptr<RenderParticle>>&
RenderParticles::getParticles()
{
    return m_particles;
}

void
RenderParticles::reset()
{
    m_numParticles = 0;
}

void
RenderParticles::incrementNumOfParticles()
{
    m_numParticles++;
}

unsigned int
RenderParticles::getNumParticles()
{
    return m_numParticles;
}

unsigned int
RenderParticles::getMaxNumParticles()
{
    return m_maxNumParticles;
}

void
RenderParticles::applyTranslation(const Vec3d t)
{
    LOG(WARNING) << "applyTranslation Not implemented!";
}

void
RenderParticles::applyRotation(const Mat3d r)
{
    LOG(WARNING) << "applyRotation Not implemented!";
}

void
RenderParticles::applyScaling(const double s)
{
    LOG(WARNING) << "applyScaling Not implemented!";
}
}
