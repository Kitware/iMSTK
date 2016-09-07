#ifndef IMSTKPBDOBJECT_H
#define IMSTKPBDOBJECT_H

#include "imstkSceneObject.h"
#include "imstkPbdModel.h"

#include <stdarg.h>

namespace imstk
{

///
/// \class PbdObject
///
/// \brief Base class for scene objects that move and/or deform
///
class PbdObject : public SceneObject
{
public:
    ///
    /// \brief
    ///
    PbdObject(std::string name) : SceneObject(name)
    {
        m_type = SceneObject::Type::Deformable;
    }

    ///
    /// \brief Destructor
    ///
    virtual ~PbdObject() = default;

    ///
    /// \brief
    ///
    void init(int nCons, ...);

    ///
    /// \brief
    ///
    virtual void integratePosition();

    ///
    /// \brief
    ///
    virtual void integrateVelocity();

    ///
    /// \brief
    ///
    virtual void updateGeometry();

    ///
    /// \brief
    ///
    virtual void constraintProjection();

    ///
    /// \brief
    ///
    void applyPhysicsToColliding();

    ///
    /// \brief
    ///
    void applyPhysicsToVisual();

    ///
    /// \brief Set/Get the geometry used for Physics computations
    ///
    std::shared_ptr<Geometry> getPhysicsGeometry() const;
    void setPhysicsGeometry(std::shared_ptr<Geometry> geometry);

    ///
    /// \brief Set/Get the Physics-to-Collision map
    ///
    std::shared_ptr<GeometryMap> getPhysicsToCollidingMap() const;
    void setPhysicsToCollidingMap(std::shared_ptr<GeometryMap> map);

    ///
    /// \brief Set/Get the Physics-to-Visual map
    ///
    std::shared_ptr<GeometryMap> getPhysicsToVisualMap() const;
    void setPhysicsToVisualMap(std::shared_ptr<GeometryMap> map);

    ///
    /// \brief Set/Get dynamical model
    ///
    std::shared_ptr<PositionBasedModel> getDynamicalModel() const;
    void setDynamicalModel(std::shared_ptr<PositionBasedModel> dynaModel);

    ///
    /// \brief Returns the number of degree of freedom
    ///
    size_t getNumOfDOF() const;

protected:

    std::shared_ptr<PositionBasedModel> m_pbdModel;
    std::shared_ptr<Geometry> m_physicsGeometry;

    //Maps
    std::shared_ptr<GeometryMap> m_physicsToCollidingGeomMap;   ///> Maps from Physics to collision geometry
    std::shared_ptr<GeometryMap> m_physicsToVisualGeomMap;      ///> Maps from Physics to visual geometry

    size_t numDOF; ///> Number of degree of freedom of the body in the discretized model
};

} // imstk

#endif // IMSTKPBDOBJECT_H