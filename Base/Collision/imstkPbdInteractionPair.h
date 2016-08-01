#ifndef IMSTKPBDINTERACTIONPAIR_H
#define IMSTKPBDINTERACTIONPAIR_H

#include "imstkInteractionPair.h"

#include "imstkPbdCollisionConstraint.h"
#include "imstkPbdObject.h"

namespace imstk {

class PbdInteractionPair
{
private:
    std::vector<CollisionConstraint*>    m_collisionConstraints;
    std::shared_ptr<PbdObject> first;
    std::shared_ptr<PbdObject> second;
    unsigned int maxIter;
public:
    PbdInteractionPair(std::shared_ptr<PbdObject> A, std::shared_ptr<PbdObject> B)
     : first (A), second (B)
    {

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
