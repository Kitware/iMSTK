/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkModule.h"

#include <unordered_map>
#include <thread>

namespace imstk
{
class Scene;

///
/// \class SceneManager
///
/// \brief Scene manager module manages multiple scenes and runs the active one
///
class SceneManager : public Module
{
public:
    enum class Mode
    {
        Simulation,
        Debug
    };

    SceneManager(std::string name = "SceneManager");
    ~SceneManager() override      = default;

    ///
    /// \brief Get the scene that the scene manager is managing
    ///
    std::shared_ptr<Scene> getActiveScene() const { return m_activeScene; };

    ///
    /// \brief Search for scene by name, returns nullptr if not found
    ///
    std::shared_ptr<Scene> getScene(std::string name) const
    {
        if (containsScene(name))
        {
            return m_sceneMap.at(name);
        }
        else
        {
            return nullptr;
        }
    }

    ///
    /// \brief Get the current mode
    ///
    Mode getMode() const { return m_mode; }

    ///
    /// \brief Switches the mode of the scene manager
    /// This alters the active scene
    ///
    void setMode(Mode mode);

    ///
    /// \brief Returns if the SceneManager contains the scene or not
    ///
    bool containsScene(std::string name) const;

    ///
    /// \brief Sets the currently updating scene
    ///
    void setActiveScene(std::string newSceneName);

    ///
    /// \brief Sets the currently updating scene, adds if does not exist yet
    ///
    void setActiveScene(std::shared_ptr<Scene> scene);

    ///
    /// \brief Adds scene to the manager
    ///
    void addScene(std::shared_ptr<Scene> scene);

    ///
    /// \brief Remove a scene from the manager
    ///
    void removeScene(std::string name);

    ///
    /// \brief Initialize the thread
    ///
    bool initModule() override;

    ///
    /// \brief Run the thread
    ///
    void updateModule() override;

protected:
    std::shared_ptr<Scene> m_activeScene;                               ///< Scene that is being managed
    std::unordered_map<std::string, std::shared_ptr<Scene>> m_sceneMap; // used in backend mode where m_sceneManagerMap is not used
    Mode m_mode;
    std::string m_prevCamName;
};
} // namespace imstk