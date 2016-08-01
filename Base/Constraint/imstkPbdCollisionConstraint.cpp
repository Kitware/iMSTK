#include "imstkPbdModel.h"
#include "imstkPbdCollisionConstraint.h"

namespace imstk
{

void EdgeEdgeConstraint::initConstraint( PositionBasedModel* model1, const unsigned int &pIdx1, const unsigned int &pIdx2,
                                         PositionBasedModel* model2, const unsigned int &pIdx3, const unsigned int &pIdx4)
{
    m_model1 = model1;
    m_model2 = model2;
    m_bodiesFirst[0] = pIdx1;
    m_bodiesFirst[1] = pIdx2;
    m_bodiesSecond[0] = pIdx3;
    m_bodiesSecond[1] = pIdx4;

}

bool EdgeEdgeConstraint::solvePositionConstraint()
{
    const unsigned int i0 = m_bodiesFirst[0];
    const unsigned int i1 = m_bodiesFirst[1];
    const unsigned int i2 = m_bodiesSecond[0];
    const unsigned int i3 = m_bodiesSecond[1];

    auto state1 = m_model1->getState();
    auto state2 = m_model2->getState();

    Vec3d& x0 = state1->getVertexPosition(i0);
    Vec3d& x1 = state1->getVertexPosition(i1);
    Vec3d& x2 = state2->getVertexPosition(i2);
    Vec3d& x3 = state2->getVertexPosition(i3);

    double a = (x3-x2).dot(x1-x0);
    double b = (x1-x0).dot(x1-x0);
    double c = (x0-x2).dot(x1-x0);
    double d = (x3-x2).dot(x3-x2);
    double e = a;
    double f = (x0-x2).dot(x3-x2);

    double det = a*e - d*b;
    double s = 0.5;
    double t = 0.5;
    if ( fabs(det) > 1e-12 ) {
        s = (c*e - b*f)/det;
        t = (c*d - a*f)/det;
        if (s < 0 || s > 1.0 ||
                t < 0 || t > 1.0) {
            return false;
        }
    }
    else {
        printf("WARNING: det is null \n");
    }

    Vec3d P = x0 + t*(x1-x0);
    Vec3d Q = x2 + s*(x3-x2);

    Vec3d n = Q - P;
    double l = n.norm();
    n /= l;

    const double dist = m_model1->getProximity() + m_model2->getProximity();

    if (l > dist )
        return false;

    Vec3d grad0 = -(1-t)*n;
    Vec3d grad1 = -(t)*n;
    Vec3d grad2 = (1-s)*n;
    Vec3d grad3 = (s)*n;

    const double im0 = state1->getInvMass(i0);
    const double im1 = state1->getInvMass(i1);
    const double im2 = state2->getInvMass(i2);
    const double im3 = state2->getInvMass(i3);

    double lambda = im0*grad0.squaredNorm() + im1*grad1.squaredNorm() + im2*grad2.squaredNorm() + im3*grad3.squaredNorm();

    lambda = (l - dist)/lambda;

    if (im0 > 0)
        x0 += -im0*lambda*grad0*m_model1->getContactStiffness();
    if (im1 > 0)
        x1 += -im1*lambda*grad1*m_model1->getContactStiffness();
    if (im2 > 0)
        x2 += -im2*lambda*grad2*m_model2->getContactStiffness();
    if (im3 > 0)
        x3 += -im3*lambda*grad3*m_model2->getContactStiffness();

    return true;
}

void PointTriangleConstraint::initConstraint(PositionBasedModel* model1, const unsigned int &pIdx1,
                                             PositionBasedModel* model2, const unsigned int &pIdx2, const unsigned int &pIdx3, const unsigned int &pIdx4)
{
    m_model1 = model1;
    m_model2 = model2;
    m_bodiesFirst[0] = pIdx1;
    m_bodiesSecond[0] = pIdx2;
    m_bodiesSecond[1] = pIdx3;
    m_bodiesSecond[2] = pIdx4;
}

bool PointTriangleConstraint::solvePositionConstraint()
{
    const unsigned int i0 = m_bodiesFirst[0];
    const unsigned int i1 = m_bodiesSecond[0];
    const unsigned int i2 = m_bodiesSecond[1];
    const unsigned int i3 = m_bodiesSecond[2];

    auto state1 = m_model1->getState();
    auto state2 = m_model2->getState();

    Vec3d x0 = state1->getVertexPosition(i0);

    Vec3d x1 = state2->getVertexPosition(i1);
    Vec3d x2 = state2->getVertexPosition(i2);
    Vec3d x3 = state2->getVertexPosition(i3);

    Vec3d x12 = x2 - x1;
    Vec3d x13 = x3 - x1;
    Vec3d n = x12.cross(x13);
    Vec3d x01 = x0 - x1;

    double alpha = n.dot(x12.cross(x01))/ (n.dot(n));
    double beta  = n.dot(x01.cross(x13))/ (n.dot(n));

    if (alpha < 0 || beta < 0 || alpha + beta > 1 ) {
        return false;
    }

    const double dist = m_model1->getProximity() + m_model2->getProximity();

    n.normalize();

    double l = x01.dot(n);

    if (l > dist)
        return false;

    double gamma = 1.0 - alpha - beta;
    Vec3d grad0 = n;
    Vec3d grad1 = -alpha*n;
    Vec3d grad2 = -beta*n;
    Vec3d grad3 = -gamma*n;

    const double im0 = state1->getInvMass(i0);

    const double im1 = state2->getInvMass(i1);
    const double im2 = state2->getInvMass(i2);
    const double im3 = state2->getInvMass(i3);

    double lambda = im0*grad0.squaredNorm() + im1*grad1.squaredNorm() + im2*grad2.squaredNorm() + im3*grad3.squaredNorm();

    lambda = (l - dist)/lambda;

    if (im0 > 0)
        x0 += -im0*lambda*grad0*m_model1->getContactStiffness();
    if (im1 > 0)
        x1 += -im1*lambda*grad1*m_model2->getContactStiffness();
    if (im2 > 0)
        x2 += -im2*lambda*grad2*m_model2->getContactStiffness();
    if (im3 > 0)
        x3 += -im3*lambda*grad3*m_model2->getContactStiffness();

    return true;
}

}
