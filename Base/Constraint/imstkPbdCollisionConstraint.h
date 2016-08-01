#ifndef IMSTKPBDCOLLISIONCONSTRAINT_H
#define IMSTKPBDCOLLISIONCONSTRAINT_H

#include <vector>

namespace imstk
{

class PositionBasedModel;

class CollisionConstraint
{
public:
    enum class Type
    {
       EdgeEdge,
       PointTriangle
    };
    std::vector<unsigned int> m_bodiesFirst;        // index of points for the first object
    std::vector<unsigned int> m_bodiesSecond;   // index of points for the second object
    PositionBasedModel* m_model1;
    PositionBasedModel* m_model2;
public:
    CollisionConstraint(const unsigned int& n1, const unsigned int& n2)
    {
        m_bodiesFirst.resize(n1);
        m_bodiesSecond.resize(n2);
    }
    virtual bool solvePositionConstraint()
    {
        return true;
    }
};

///
/// \brief The EdgeEdgeConstraint class for edge-edge collision response
///

class EdgeEdgeConstraint : public CollisionConstraint
{

public:
    EdgeEdgeConstraint() : CollisionConstraint(2,2) {}

    Type getType() const { return Type::EdgeEdge; }
    ///
    /// \brief initialize constraint
    /// \param pIdx1 first point of the edge from object1
    /// \param pIdx2 second point of the edge from object1
    /// \param pIdx3 first point of the edge from object2
    /// \param pIdx4 second point of the edge from object2
    /// \return  true if succeeded
    ///
    void initConstraint( PositionBasedModel* model1, const unsigned int& pIdx1, const unsigned int& pIdx2,
                         PositionBasedModel* model2, const unsigned int& pIdx3, const unsigned int& pIdx4);

    bool solvePositionConstraint();
};

///
/// \brief The PointTriangleConstraint class for point-triangle collision response
///


class PointTriangleConstraint : public CollisionConstraint
{

public:
    PointTriangleConstraint() : CollisionConstraint(1,3) {}

    Type getType() const { return Type::PointTriangle; }
    ///
    /// \brief initialize constraint
    /// \param pIdx1 index of the point from object1
    /// \param pIdx2 first point of the triangle from object2
    /// \param pIdx3 second point of the triangle from object2
    /// \param pIdx4 third point of the triangle from object2
    /// \return
    ///
    void initConstraint( PositionBasedModel* model1, const unsigned int& pIdx1,
                         PositionBasedModel* model2, const unsigned int& pIdx2,  const unsigned int& pIdx3, const unsigned int& pIdx4);

    bool solvePositionConstraint();
};

}


#endif // IMSTKPBDCOLLISIONCONSTRAINT_H
