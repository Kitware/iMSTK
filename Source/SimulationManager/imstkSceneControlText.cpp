/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkSceneControlText.h"
#include "imstkSceneManager.h"

namespace imstk
{
SceneControlText::SceneControlText(const std::string& name) : TextVisualModel(name)
{
    // Create visual model
    setFontSize(40);
    setVisibility(false);
    setText("Simulation Paused\nPress Space to Continue\n"
        "Press R to Reset\nPress C to clear pause screen");

    setPosition(TextVisualModel::DisplayPosition::CenterCenter);
}

void
SceneControlText::init()
{
    setVisibility(m_useTextStatus ? m_sceneManager.lock()->getPaused() : false);
}
} // namespace imstk