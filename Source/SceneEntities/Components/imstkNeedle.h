/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"
#include "imstkMath.h"

#include <unordered_map>
#include <unordered_set>

namespace imstk
{
class Entity;
class LineMesh;

///
/// \brief Punctures are identified via three ints.
/// The needle id, the puncturable id, and a local id that allows multi punctures
/// on the needle,puncture pair. This could be a cell (face) id, vertex id, or
/// an index into some other structure.
///
using PunctureId = std::tuple<int, int, int>;

///
/// \brief The puncture itself is composed of a state and extra
/// non-essential user data.
///
struct Puncture
{
    public:
        enum class State
        {
            REMOVED,
            TOUCHING,
            INSERTED
        };
        ///
        /// \brief Accompanying data stored with a puncture. No usage within Needle
        /// structure.
        ///
        struct UserData
        {
            int id = -1;
            int ids[4];
            Vec4d weights = Vec4d::Zero();
        };

    public:
        //PunctureId id;
        State state = State::REMOVED;
        UserData userData;
};

///
/// \struct PunctureIdHash
///
/// \brief The entity and tissue id should be reversible
///
struct PunctureIdHash
{
    std::size_t operator()(const imstk::PunctureId& k) const
    {
        // Swapping 0 and 1 should result in equivalence.
        return cantor(symCantor(std::get<0>(k), std::get<1>(k)), std::get<2>(k));
    }
};
///
/// \struct PunctureIdEq
///
/// \brief The entity and tissue id should be reversible
///
struct PunctureIdEq
{
    bool operator()(const imstk::PunctureId& a, const imstk::PunctureId& b) const
    {
        std::pair<int, int> a1 = { std::get<0>(a), std::get<1>(a) };
        if (a1.second < a1.first)
        {
            std::swap(a1.first, a1.second);
        }
        std::pair<int, int> b1 = { std::get<0>(b), std::get<1>(b) };
        if (b1.second < b1.first)
        {
            std::swap(b1.first, b1.second);
        }
        return (a1.first == b1.first) && (a1.second == b1.second) && (std::get<2>(a) == std::get<2>(b));
    }
};

using PunctureMap = std::unordered_map<PunctureId, std::shared_ptr<Puncture>, PunctureIdHash, PunctureIdEq>;

///
/// \class Needle
///
/// \brief Base for all needles in imstk it supports global puncture state,
/// per object puncture state, and per id/cell puncture state
///
class Needle : public Component
{
public:
    Needle(const std::string& name = "Needle") : Component(name) { }
    ~Needle() override = default;

    ///
    /// \brief Get/Set puncture data
    /// @{
    void setPuncture(const PunctureId& id, std::shared_ptr<Puncture> data);
    std::shared_ptr<Puncture> getPuncture(const PunctureId& id);
    /// @}

    ///
    /// \brief Get/set puncture state. This can be done through data too
    /// but this supports the allocation of new puncture data should you
    /// query a non-existent puncture.
    /// @{
    void setState(const PunctureId& id, const Puncture::State state);
    Puncture::State getState(const PunctureId& id);
    /// @}

    const PunctureMap& getPunctures() const { return m_punctures; }

    ///
    /// \brief Get if inserted at all
    ///
    bool getInserted() const;

    ///
    /// \brief Returns direction at needle tip.
    ///
    virtual Vec3d getNeedleDirection() const { return Vec3d(1.0, 0.0, 0.0); }
    virtual Vec3d getNeedleTip() const { return Vec3d(0.0, 0.0, 0.0); }

protected:
    PunctureMap m_punctures;
};

class StraightNeedle : public Needle
{
public:
    StraightNeedle(const std::string& name = "StraightNeedle") : Needle(name) { }
    ~StraightNeedle() override = default;

    void setNeedleGeometry(std::shared_ptr<LineMesh> straightNeedleGeom) { m_needleGeom = straightNeedleGeom; }
    std::shared_ptr<LineMesh> getNeedleGeometry() const { return m_needleGeom; }

    ///
    /// \brief Returns axes/direciton of the needle
    ///
    Vec3d getNeedleDirection() const override;

    ///
    /// \brief Returns tip of the needle
    ///
    Vec3d getNeedleTip() const override { return getNeedleStart(); }

    const Vec3d& getNeedleStart() const;
    const Vec3d& getNeedleEnd() const;

protected:
    std::shared_ptr<LineMesh> m_needleGeom;
};

class ArcNeedle : public Needle
{
public:
    ArcNeedle(const std::string& name = "ArcNeedle") : Needle(name)
    {
        const Mat4d rot = mat4dRotation(Rotd(-PI_2, Vec3d(0.0, 1.0, 0.0))) *
                          mat4dRotation(Rotd(-0.6, Vec3d(1.0, 0.0, 0.0)));

        // Manually setup an arc aligned with the geometry, some sort of needle+arc generator
        // could be a nice addition to imstk
        Mat3d arcBasis = Mat3d::Identity();
        arcBasis.col(0) = Vec3d(0.0, 0.0, -1.0);
        arcBasis.col(1) = Vec3d(1.0, 0.0, 0.0);
        arcBasis.col(2) = Vec3d(0.0, 1.0, 0.0);
        arcBasis = rot.block<3, 3>(0, 0) * arcBasis;
        const Vec3d  arcCenter = (rot * Vec4d(0.0, -0.005455, 0.008839, 1.0)).head<3>();
        const double arcRadius = 0.010705;
        setArc(arcCenter, arcBasis, arcRadius, 0.558, 2.583);
    }

    ~ArcNeedle() override = default;

    /*void setNeedleGeometry(std::shared_ptr<LineMesh> straightNeedleGeom) { m_needleGeom = straightNeedleGeom; }
    std::shared_ptr<LineMesh> getNeedleGeometry() const { return m_needleGeom; }*/

    ///
    /// \brief Returns axes/direciton of the needle
    ///
    //Vec3d getNeedleDirection() const override;

    ///
    /// \brief Returns tip of the needle
    ///
    //Vec3d getNeedleTip() const override { return getNeedleStart(); }

    /*const Vec3d& getNeedleStart() const;
    const Vec3d& getNeedleEnd() const;*/

    void setArc(const Vec3d& arcCenter, const Mat3d& arcBasis,
                double arcRadius, double beginRad, double endRad)
    {
        m_arcCenter = arcCenter;
        m_arcBasis  = arcBasis;
        m_beginRad  = beginRad;
        m_endRad    = endRad;
        m_arcRadius = arcRadius;
    }

    ///
    /// \brief Get the basis post transformation of the rigid body
    ///
    const Mat3d& getArcBasis() const { return m_arcBasis; }

    ///
    /// \brief Get the arc center post transformation of the rigid body
    ///
    const Vec3d& getArcCenter() { return m_arcCenter; }
    const double getBeginRad() const { return m_beginRad; }
    const double getEndRad() const { return m_endRad; }
    const double getArcRadius() const { return m_arcRadius; }

protected:
    std::shared_ptr<LineMesh> m_needleGeom;

    Mat3d  m_arcBasis  = Mat3d::Identity();
    Vec3d  m_arcCenter = Vec3d::Zero();
    double m_arcRadius = 1.0;
    double m_beginRad  = 0.0;
    double m_endRad    = PI * 2.0;
};

///
/// \class Puncturable
///
/// \brief Place this on an object to make it puncturable by a needle. This allows
/// puncturables to know they've been punctured without needing to be aware of the
/// needle.
/// The Puncturable supports multiple local ids
///
class Puncturable : public Component
{
public:
    Puncturable(const std::string& name = "Puncturable") : Component(name) { }
    ~Puncturable() override = default;

    ///
    /// \brief Get/Set puncture data
    /// @{
    void setPuncture(const PunctureId& id, std::shared_ptr<Puncture> data);
    std::shared_ptr<Puncture> getPuncture(const PunctureId& id);
    /// @}

    bool getPunctured() const;

protected:
    PunctureMap m_punctures;
};

///
/// \brief Get puncture id between needle and puncturable
///
PunctureId getPunctureId(std::shared_ptr<Needle>      needle,
                         std::shared_ptr<Puncturable> puncturable,
                         const int                    supportId = -1);
} // namespace imstk