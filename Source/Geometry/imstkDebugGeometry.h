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

#include "imstkMath.h"
#include "imstkRenderMaterial.h"
#include <memory>

namespace imstk
{
///
/// \class imstkDbgRenderGeometry
///
/// \brief debug render geometry base class
///
class DebugRenderGeometry
{
public:
    ///
    /// \brief Enumeration for the debug geometry type
    ///
    enum class Type
    {
        Points,
        Lines,
        Triangles,
        Tetrahedra
    };

    ///
    /// \brief Clear the memory and set the count to zero
    ///
    void clear();

    ///
    /// \brief Returns the vertex positions that are used for debug line rendering
    ///
    const StdVectorOfVec3d& getVertexPositions() { return m_vertices; };
    StdVectorOfVec3d& getVertexPositionsNonConst() { return m_vertices; };

    ///
    /// \brief Set the vertex data
    ///
    void setVertexData(const StdVectorOfVec3d& verts);

    ///
    /// \brief Get the name
    ///
    const std::string& getName() const { return m_name; };

    ///
    /// \brief Get type
    ///
    const Type& getType() const { return m_type; };

    ///
    /// \brief
    ///
    bool isModified() { return m_isModified; }
    void setDataModifiedFlag(const bool flag) { m_isModified = flag; };

    ///
    /// \brief Set/Get render material
    ///
    void setRenderMaterial(std::shared_ptr<RenderMaterial> renderMat);
    std::shared_ptr<RenderMaterial> getRenderMaterial() const;

protected:
    friend class VTKRenderer;
    friend class VTKRenderDelegate;

    ///
    /// \brief Constructor
    ///
    DebugRenderGeometry(const std::string& name, Type type, const unsigned int size = 300) :
        m_name(name), m_type(type), m_dataSize(size)
    {
        allocate(size);
        m_renderMaterial = std::make_shared<RenderMaterial>();
    }

    ///
    /// \brief Preallocate a fixed size for the debug rendering data
    ///
    virtual void allocate(const unsigned int size)
    {
        m_vertices.resize(size);
    };

    Type m_type;                 ///> Debug geometry type
    StdVectorOfVec3d m_vertices; ///> Vertex data
    size_t m_dataSize = 0;       ///> Count for data size

    std::string m_name; ///> Custom name of the scene object
    bool m_renderDelegateCreated = false;
    bool m_isModified = false;

    std::shared_ptr<RenderMaterial> m_renderMaterial = nullptr; ///> Render material
};

///
/// \class imstkDbgRenderPoints
///
/// \brief debug points to render
///
class DebugRenderPoints : public DebugRenderGeometry
{
public:
    ///
    /// \brief Constructor/destructor
    ///
    DebugRenderPoints(const std::string& name, const unsigned int size) : DebugRenderGeometry(name, Type::Points, size){}
};

///
/// \class DbgRenderLines
///
/// \brief List of disjoint lines to render
///
class DebugRenderLines : public DebugRenderGeometry
{
public:
    ///
    /// \brief Constructor/destructor
    ///
    DebugRenderLines(const std::string& name, const unsigned int size = 300) : DebugRenderGeometry(name, Type::Lines, size){}
};

///
/// \class imstkDbgRenderTriangles
///
/// \brief debug triangles to render
///
class DebugRenderTriangles : public DebugRenderGeometry
{
public:
    ///
    /// \brief Constructor/destructor
    ///
    DebugRenderTriangles(const std::string& name, const unsigned int size = 3*100) : DebugRenderGeometry(name, Type::Triangles, size){}

protected:
    ///
    /// \brief Preallocate a fixed size for the debug rendering data
    ///
    void allocate(const unsigned int size) override
    {
        if (size % 3 == 0)
        {
            m_vertices.reserve(size);
        }
        else
        {
            LOG(WARNING) << "WARNING: The size of the triangle array should be multiples of three!";
        }
    }
};
}
