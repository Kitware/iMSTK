#include "imstkPbdCollisionConstraint.h"
#include "g3log/g3log.hpp"

namespace imstk
{

PbdCollisionConstraint::PbdCollisionConstraint(const unsigned int& n1, const unsigned int& n2)
{
    m_bodiesFirst.resize(n1);
    m_bodiesSecond.resize(n2);
}

} // imstk