#ifndef imstkPbdObject_h
#define imstkPbdObject_h

#include "imstkDynamicObject.h"
#include "imstkDynamicalModel.h"
#include "imstkPbdModel.h"

#include <stdarg.h>

namespace imstk
{

class Geometry;
class GeometryMap;

///
/// \class PbdObject
///
/// \brief Base class for scene objects that move and/or deform under position
/// based dynamics formulation
///
class PbdObject : public DynamicObject<PbdState>
{
public:
    ///
    /// \brief Constructor
    ///
    PbdObject(std::string name) : DynamicObject(name)
    {
        m_type = SceneObject::Type::Pbd;
    }

    ///
    /// \brief Destructor
    ///
    virtual ~PbdObject() = default;

    ///
    /// \brief Initialize the pbd configuration
    /// TODO: Parse from config file
    ///
    bool initialize(int nCons, ...);

    ///
    /// \brief Update the position based on Verlet time stepping rule
    ///
    virtual void integratePosition();

    ///
    /// \brief Update the velocity
    ///
    virtual void updateVelocity();

    ///
    /// \brief Solve the pbd constraints by projection
    ///
    virtual void solveConstraints();

protected:

    std::shared_ptr<PbdModel> m_pbdModel; ///> PBD mathematical model
};

} // imstk

#endif // imstkPbdObject_h
