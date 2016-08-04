#ifndef IMSTKPBDINTERACTIONPAIR_H
#define IMSTKPBDINTERACTIONPAIR_H

#include "imstkInteractionPair.h"

#include "imstkPbdCollisionConstraint.h"
#include "imstkPbdObject.h"

namespace imstk
{

///
/// \class PbdInteractionPair
///
/// \brief
///
class PbdInteractionPair
{
public:
    ///
    /// \brief Constructor
    ///
    PbdInteractionPair(std::shared_ptr<PbdObject> A, std::shared_ptr<PbdObject> B)
     : first (A), second (B)
    {

    }

    ///
    /// \brief
    ///
    inline void resetConstraints()
    {
        m_collisionConstraints.clear();
    }

    ///
    /// \brief
    ///
    inline void setNumberOfInterations(const unsigned int& n)
    {
        maxIter = n;
    }

    ///
    /// \brief
    ///
    bool doBroadPhase();

    ///
    /// \brief
    ///
    void doNarrowPhase();

    ///
    /// \brief
    ///
    void doCollision();

private:
    std::vector<CollisionConstraint*>    m_collisionConstraints;
    std::shared_ptr<PbdObject> first;
    std::shared_ptr<PbdObject> second;
    unsigned int maxIter;
};

}

#endif // IMSTKPBDINTERACTIONPAIR_H