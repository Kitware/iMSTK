/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVisualModel.h"
#include "imstkMath.h"

namespace imstk
{
///
/// \class AxesModel
///
/// \brief Defines an axes that should be rendered
///
class AxesModel : public VisualModel
{
public:
    AxesModel(const std::string& name = "AxesModel") : VisualModel(name)
    {
        setDelegateHint("AxesModel");
    }

    const Vec3d& getPosition() const { return m_axesPos; }
    void setPosition(const Vec3d& axesPos) { m_axesPos = axesPos; }

    const Quatd& getOrientation() const { return m_axesOrientation; }
    void setOrientation(const Quatd& axesOrientation) { m_axesOrientation = axesOrientation; }

    const Vec3d& getScale() const { return m_axesScale; }
    void setScale(const Vec3d& axesScale) { m_axesScale = axesScale; }

    bool getAxesLabelsVisible() const { return m_axesLabelsVisible; }
    void setAxesLabelsVisible(const bool visible) { m_axesLabelsVisible = visible; }

protected:
    Vec3d m_axesPos = Vec3d::Zero();
    Quatd m_axesOrientation   = Quatd::Identity();
    Vec3d m_axesScale         = Vec3d::Ones();
    bool  m_axesLabelsVisible = false;
};
} // namespace imstk