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
/// \brief Debug render geometry base class
/// During rendering, firstly call to clear() to clear the vertex buffer then iteratively call to appendVertex()
/// Another way to modify rendering data is to call resizeBuffer(), then fill data by calling to setVertex() function
/// In both ways, after finishing data modification, triggerDataModified() must be called to notify the render backend
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
    /// \brief Clear the vertex buffer
    ///
    void clear() { resizeBuffer(0); }

    ///
    /// \brief Reserve memory for fast push_back
    ///
    virtual void reserve(const size_t size) { m_VertexBuffer.reserve(size); }

    ///
    /// \brief Returns the vertex position buffer that is used for debug rendering
    ///
    const StdVectorOfVec3d& getVertexBuffer() const { return m_VertexBuffer; }
    double* getVertexBufferPtr() { return reinterpret_cast<double*>(m_VertexBuffer.data()); }
    const double* getVertexBufferPtr() const { return reinterpret_cast<const double*>(m_VertexBuffer.data()); }

    ///
    /// \brief Returns the number of the vertices in the buffer
    ///
    size_t getNumVertices() const { return m_VertexBuffer.size(); }

    ///
    /// \brief Resize the vertex buffer
    ///
    void resizeBuffer(const size_t newSize) { m_VertexBuffer.resize(newSize); }

    ///
    /// \brief Return the vertex at the given idx
    ///
    const Vec3d& getVertex(const size_t idx) const;

    ///
    /// \brief Set the vertex at index idx
    ///
    void setVertex(const size_t idx, const Vec3d& vert);

    ///
    /// \brief Append a vertex
    ///
    void appendVertex(const Vec3d& vert) { m_VertexBuffer.push_back(vert); }

    ///
    /// \brief Get the name of debug geometry
    ///
    const std::string& getName() const { return m_name; }

    ///
    /// \brief Get rendering geometry type
    ///
    const Type& getType() const { return m_type; }

    ///
    /// \brief Return m_isModified flag, used to communicate with the render engine
    /// For performance reason, the only way to change this flag is to manually call to turnDataModifiedFlagON() or turnDataModifiedFlagOFF
    /// Thus, we must call turnDataModifiedFlagON() explicity after finished data manipulation
    ///
    bool isModified() const { return m_isModified; }
    void turnDataModifiedFlagON() { m_isModified = true; }
    void turnDataModifiedFlagOFF() { m_isModified = false; }

    ///
    /// \brief Set/Get render material
    ///
    void                                   setRenderMaterial(const std::shared_ptr<RenderMaterial>& renderMat);
    const std::shared_ptr<RenderMaterial>& getRenderMaterial() const;

protected:
    friend class VTKRenderer;
    friend class VTKRenderDelegate;

    ///
    /// \brief Constructor
    ///
    DebugRenderGeometry(const std::string& name, const Type type) :
        m_name(name), m_type(type), m_renderMaterial(std::make_shared<RenderMaterial>())
    {
    }

    virtual ~DebugRenderGeometry() = default;

    std::string m_name;                   ///> Custom name of the scene object
    Type m_type;                          ///> Debug geometry type
    StdVectorOfVec3d m_VertexBuffer;      ///> Vertex buffer

    bool m_renderDelegateCreated = false; ///> This variable is used in Renderer
    bool m_isModified = false;
    std::shared_ptr<RenderMaterial> m_renderMaterial = nullptr;
};

///
/// \class DebugRenderPoints
///
/// \brief Debug points to render
///
class DebugRenderPoints : public DebugRenderGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    DebugRenderPoints(const std::string& name) : DebugRenderGeometry(name, Type::Points) {}
};

///
/// \class DebugRenderLines
///
/// \brief List of disjoint lines to render
///
class DebugRenderLines : public DebugRenderGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    DebugRenderLines(const std::string& name) : DebugRenderGeometry(name, Type::Lines) {}

    ///
    /// \brief Reserve memory for fast push_back
    ///
    virtual void reserve(const size_t size) override { m_VertexBuffer.reserve(size * 2); }
};

///
/// \class DebugRenderTriangles
///
/// \brief Debug triangles to render
///
class DebugRenderTriangles : public DebugRenderGeometry
{
public:
    ///
    /// \brief Constructor
    ///
    DebugRenderTriangles(const std::string& name) : DebugRenderGeometry(name, Type::Triangles) {}

    ///
    /// \brief Reserve memory for fast push_back
    ///
    virtual void reserve(const size_t size) override { m_VertexBuffer.reserve(size * 3); }
};
}
