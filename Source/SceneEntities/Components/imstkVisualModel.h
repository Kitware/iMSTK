/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkComponent.h"
#include "imstkEventObject.h"

#include <memory>
#include <string>
#include <unordered_map>

namespace imstk
{
class Geometry;
class RenderMaterial;
class Renderer;

///
/// \class VisualModel
///
/// \brief Contains geometric, material, and render information
///
class VisualModel : public Component, public EventObject
{
public:
    VisualModel(const std::string& name = "VisualModel");
    ~VisualModel() override = default;

    // *INDENT-OFF*
    SIGNAL(VisualModel, modified);
    // *INDENT-ON*

    ///
    /// \brief Get/Set geometry
    ///@{
    std::shared_ptr<Geometry> getGeometry() const { return m_geometry; }
    void setGeometry(std::shared_ptr<Geometry> geometry) { m_geometry = geometry; }
    ///@}

    ///
    /// \brief Get/Set name
    ///@{
    const std::string& getName() const { return m_name; }
    void setName(std::string name) { m_name = name; }
    ///@}

    ///
    /// \brief Get/Set the delegate hint, which helps indicate
    /// how to render this VisualModel
    ///@{
    const std::string getDelegateHint() const;
    void setDelegateHint(const std::string& name) { m_delegateHint = name; }
    ///@}

    ///
    /// \brief Set/Get render material
    ///@{
    void setRenderMaterial(std::shared_ptr<RenderMaterial> renderMaterial);
    std::shared_ptr<RenderMaterial> getRenderMaterial() const { return m_renderMaterial; }
    ///@}

    ///
    /// \brief Visibility functions
    ///@{
    void show() { setIsVisible(true); }
    void hide() { setIsVisible(false); }
    bool isVisible() const { return m_isVisible; }
    void setIsVisible(const bool visible);
    ///@}

    ///
    /// \brief Get/Set whether the delegate has been created
    ///@{
    bool getRenderDelegateCreated(Renderer* ren);
    void setRenderDelegateCreated(Renderer* ren, bool created) { m_renderDelegateCreated[ren] = created; }
    ///@}

    void postModified() { this->postEvent(Event(VisualModel::modified())); }

protected:
    std::string m_delegateHint;

    std::shared_ptr<Geometry>       m_geometry;
    std::shared_ptr<RenderMaterial> m_renderMaterial;

    bool m_isVisible; ///< true if mesh is shown, false if mesh is hidden
    std::unordered_map<Renderer*, bool> m_renderDelegateCreated;
};
} // namespace imstk
