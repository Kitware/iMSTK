#ifndef IMSTKPBDINTERACTIONPAIR_H
#define IMSTKPBDINTERACTIONPAIR_H

#include "imstkInteractionPair.h"

#include "imstkPbdCollisionConstraint.h"
#include "imstkPbdCollidingObject.h"

namespace imstk {

class PbdInteractionPair : public InteractionPair
{
private:
    std::vector<CollisionConstraint*>    m_collisionConstraints;
    std::shared_ptr<PbdCollidingObject> first;
    std::shared_ptr<PbdCollidingObject> second;
    unsigned int maxIter;
public:
    PbdInteractionPair(std::shared_ptr<CollidingObject> A,
                       std::shared_ptr<CollidingObject> B,
                       CollisionDetection::Type CDType,
                       CollisionHandling::Type CHAType,
                       CollisionHandling::Type CHBType)
    {
        first = std::static_pointer_cast<PbdCollidingObject>(A);
        second = std::static_pointer_cast<PbdCollidingObject>(B);
    }

    inline void resetConstraints()
    {
        m_collisionConstraints.clear();
    }

    inline void setNumberOfInterations(const unsigned int& n) { maxIter = n; }

    bool doBroadPhase();

    void doNarrowPhase();

    void doCollision();

};

}

#endif // IMSTKPBDINTERACTIONPAIR_H
