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

#ifndef imstkGeometry_h
#define imstkGeometry_h

#include "g3log/g3log.hpp"
#include "imstkMath.h"

namespace imstk {

///
/// \class Geometry
///
/// \brief Base class for any geometrical representation
///
class Geometry
{
public:
    enum class Type
    {
        Plane,
        Sphere,
        Cube,
        SurfaceMesh,
        TetrahedralMesh,
        HexahedralMesh
    };

    ///
    /// \brief Destructor
    ///
    virtual ~Geometry() {}

    ///
    /// \brief Print
    ///
    virtual void print() const;

    ///
    /// \brief Returns the volume of the geometry (if valid)
    ///
    virtual double getVolume() const = 0;

    ///
    /// \brief Translate the geometry in Cartesian space
    ///
    void         translate(const Vec3d& t);
    void         translate(const double& x,
                           const double& y,
                           const double& z);

    ///
    /// \brief Rotate the geometry in Cartesian space
    ///
    void         rotate(const Quatd& r);
    void         rotate(const Mat3d& r);
    void         rotate(const Vec3d & axis,
                        const double& angle);

    ///
    /// \brief Scale in Cartesian directions
    ///
    void         scale(const double& scaling);

    ///
    /// \brief Applies a rigid transform to the geometry
    ///
    void        transform(const RigidTransform3d& transform);

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    bool isMesh() const;

    // Accessors

    ///
    /// \brief Get/Set position
    ///
    const Vec3d& getPosition() const;
    void         setPosition(const Vec3d& position);
    void         setPosition(const double& x,
                             const double& y,
                             const double& z);

    ///
    /// \brief Get/Set orientation
    ///
    const Quatd       & getOrientation() const;
    void                setOrientation(const Quatd& orientation);
    void                setOrientation(const Mat3d& orientation);
    void                setOrientation(const Vec3d & axis,
                                       const double& angle);
    ///
    /// \brief Get/Set scaling
    ///
    const double      & getScaling() const;
    void                setScaling(const double& scaling);

    ///
    /// \brief Returns the type of the geometry
    ///
    const Type& getType() const;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    const std::string getTypeName() const;
protected:

    ///
    /// \brief Constructor
    ///
    Geometry(Type type,
             const Vec3d& position = WORLD_ORIGIN,
             const Quatd& orientation = Quatd::Identity()) :
        m_type(type),
        m_position(position),
        m_orientation(orientation){}

    Type m_type; ///> Geometry type
    Vec3d  m_position; ///> position
    Quatd  m_orientation; ///> orientation
    double m_scaling = 1; ///> Scaling
};

} //imstk

#endif // ifndef imstkGeometry_h
