/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"

#pragma once

namespace imstk
{
class PbdObjectController;
class VisualModel;

///
/// \class ObjectControllerGhost
///
/// \brief A behaviour that renders a second copy of the controlled object
/// at a lower opacity in the physical position as opposed to the original
/// rendered at the virtual
/// \todo: Device render mode to render the actual arm and body of the device
/// \todo: This should be simplified when PbdObject is removed
///
class ObjectControllerGhost : public SceneBehaviour
{
public:
    ObjectControllerGhost(const std::string& name = "ObjectControllerGhost");

    void setController(std::shared_ptr<PbdObjectController> controller) { m_pbdController = controller; }
    std::shared_ptr<PbdObjectController> getPbdController() const { return m_pbdController; }

    std::shared_ptr<VisualModel> getGhostModel() const { return m_ghostVisualModel; }

    ///
    /// \brief Get/Set whether to use force fade or not. Force fade sets
    /// opacity of ghost geometry according to force. Low haptic force
    /// results in transparent geometry, high force makes it opaque.
    ///@{
    void setUseForceFade(bool useForceFade) { m_useForceFade = useForceFade; }
    bool getUseForceFade() const { return m_useForceFade; }
    ///@}

    void visualUpdate(const double& dt) override;

protected:
    void init() override;

protected:
    std::shared_ptr<PbdObjectController> m_pbdController = nullptr;
    std::shared_ptr<VisualModel> m_ghostVisualModel      = nullptr;
    bool m_useForceFade = false;
};
} // namespace imstk