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

#ifndef IMSTKPBDCONSTRAINT_H
#define IMSTKPBDCONSTRAINT_H

#include "imstkMath.h"

const double EPS = 1e-6;

namespace imstk
{

class PositionBasedModel;

///
/// \brief Based Constraint class for Position based dynamics
///
class PbdConstraint
{
public:
    enum class Type
    {
        Distance,
        Dihedral,
        Area,
        Volume,
        FEMTet,
        FEMHex
    };

    ///
    /// \brief Constructor
    ///
    PbdConstraint(const unsigned int nP)
    {
        m_bodies.resize(nP);
    }

    ///
    /// \brief abstract interface to know the type of constraint
    /// \return particular type
    ///
    virtual Type getType() const = 0;

    ///
    /// \brief update constraint
    /// \param model \class PositionBasedModel
    /// \return true if succeeded
    ///
    virtual bool updateConstraint(PositionBasedModel &model)
    {
        return true;
    }

    ///
    /// \brief compute delta position from the constraint function
    /// \param model \class PositionBasedModel
    /// \return true if succeeded
    ///
    virtual bool solvePositionConstraint(PositionBasedModel &model)
    {
        return true;
    }

    ///
    /// \brief compute delta velocity, specifically for rigid bodies
    /// \param model \class PositionBasedModel
    /// \return true if succeeded
    ///
    virtual bool solveVelocityConstraint(PositionBasedModel &model)
    {
        return true;
    }

public:
    std::vector<unsigned int> m_bodies; // index of points for the constraint
};

///
/// \class DistanceConstraint
///
/// \brief
///
class DistanceConstraint : public PbdConstraint
{
public:
    ///
    /// \brief
    ///
    DistanceConstraint() : PbdConstraint(2) {}

    ///
    /// \brief
    ///
    Type getType() const { return Type::Distance; }

    ///
    /// \brief
    ///
    void initConstraint(PositionBasedModel& model, const unsigned int& pIdx1,
        const unsigned int& pIdx2, const double k = 1e-1);

    ///
    /// \brief
    ///
    bool solvePositionConstraint(PositionBasedModel &model);

public:
    double m_restLength;
    double m_stiffness;
};

///
/// \class DihedralConstraint
///
/// \brief
///
class DihedralConstraint : public PbdConstraint
{
public:
    ///
    /// \brief
    ///
    DihedralConstraint() : PbdConstraint(4) {}
    ///
    /// \brief
    ///
    Type getType() const { return Type::Dihedral; }

    ///
    /// \brief initConstraint
    ///        p3
    ///       / | \
    ///      /  |  \
    ///     p0  |  p1
    ///      \  |  /
    ///       \ | /
    ///         p2
    /// \param model
    /// \param pIdx1 index of p0
    /// \param pIdx2 index of p1
    /// \param pIdx3 index of p2
    /// \param pIdx4 index of p3
    /// \param k stiffness
    ///
    void initConstraint(PositionBasedModel& model, const unsigned int& pIdx1, const unsigned int& pIdx2,
                        const unsigned int& pIdx3, const unsigned int& pIdx4, const double k = 1e-3 );

    ///
    /// \brief
    ///
    bool solvePositionConstraint(PositionBasedModel &model);

public:
    double m_restAngle;
    double m_stiffness;
};

///
/// \class AreaConstraint
///
/// \brief
///
class AreaConstraint : public PbdConstraint
{
public:
    ///
    /// \brief
    ///
    AreaConstraint() : PbdConstraint(3)
    {}

    ///
    /// \brief
    ///
    Type getType() const
    {
        return Type::Area;
    }

    ///
    /// \brief
    ///
    void initConstraint(PositionBasedModel& model, const unsigned int& pIdx1,
        const unsigned int& pIdx2, const unsigned int& pIdx3, const double k = 2.5);

    ///
    /// \brief
    ///
    bool solvePositionConstraint(PositionBasedModel &model);

public:
    double m_restArea;
    double m_stiffness;
};

///
/// \class VolumeConstraint
///
/// \brief
///
class VolumeConstraint : public PbdConstraint
{
public:
    ///
    /// \brief
    ///
    VolumeConstraint() : PbdConstraint(4)
    {}

    ///
    /// \brief
    ///
    Type getType() const
    {
        return Type::Volume;
    }

    ///
    /// \brief
    ///
    void initConstraint(PositionBasedModel& model, const unsigned int& pIdx1, const unsigned int& pIdx2,
                        const unsigned int& pIdx3, const unsigned int& pIdx4, const double k = 2.0 );

    ///
    /// \brief
    ///
    bool solvePositionConstraint(PositionBasedModel &model);

public:
    double m_restVolume;
    double m_stiffness;
};

///
/// \class FEMConstraint
///
/// \brief The FEMConstraint class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
/// We provide several model for elastic energy including:
/// Linear, Corrotation, St Venant-Kirchhof and NeoHookean
///
class FEMConstraint : public PbdConstraint
{
public:
    enum class MaterialType
    {
        Linear,
        Corotation,
        StVK,
        NeoHookean
    } m_material;

    ///
    /// \brief
    ///
    explicit FEMConstraint(const unsigned int nP, MaterialType mtype = MaterialType::StVK) :
        PbdConstraint(nP) , m_material(mtype)
    {}

public:
    double m_Volume;
    Eigen::Matrix3d m_invRestMat;
};

///
/// \class FEMTetConstraint
///
/// \brief The FEMTetConstraint class class for constraint as the elastic energy
/// computed by linear shape functions with tetrahedral mesh.
///
class FEMTetConstraint : public  FEMConstraint
{
public:
    ///
    /// \brief
    ///
    explicit FEMTetConstraint( MaterialType mtype = MaterialType::StVK) :
        FEMConstraint(4, mtype)
    {}

    ///
    /// \brief
    ///
    Type getType() const
    {
        return Type::FEMTet;
    }

    ///
    /// \brief
    ///
    bool initConstraint(PositionBasedModel& model, const unsigned int& pIdx1, const unsigned int& pIdx2,
                        const unsigned int& pIdx3, const unsigned int& pIdx4);

    ///
    /// \brief
    ///
    bool solvePositionConstraint(PositionBasedModel &model);
};

///
/// \class FEMHexConstraint
///
/// \brief The FEMHexConstraint class class for constraint as the elastic energy
/// computed by linear shape functions with hexahedral mesh.
///
class FEMHexConstraint : public  FEMConstraint
{
public:
    ///
    /// \brief
    ///
    explicit FEMHexConstraint( MaterialType mtype = MaterialType::StVK) :
        FEMConstraint(8, mtype)
    {}

    ///
    /// \brief
    ///
    Type getType() const
    {
        return Type::FEMHex;
    }

    ///
    /// \brief
    ///
    bool initConstraint(PositionBasedModel& model, const unsigned int& pIdx1,
                        const unsigned int& pIdx2, const unsigned int& pIdx3,
                        const unsigned int& pIdx4, const unsigned int& pIdx5,
                        const unsigned int& pIdx6, const unsigned int& pIdx7,
                        const unsigned int& pIdx8);

    ///
    /// \brief
    ///
    bool solvePositionConstraint(PositionBasedModel &model);
};

}

#endif // IMSTKPBDCONSTRAINT_H