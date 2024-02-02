/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkComponent.h"
#include "imstkMath.h"

#pragma once

namespace imstk
{
class PbdObject;
class TaskNode;
class CellPicker;
class AnalyticalGeometry;

///
/// \class Burner
///
/// \brief Defines the behaviour to allow a tool to burn a pbdObject.  This is done
/// by storing state on the mesh that defines the damage from burning and the visual change
/// caused by cauterization. These are currently normalized from [0,1] where 1 is the maximum
/// damage. Once the damage reaches 1, the cell is deleted on the next visual update.
///
///
class Burner : public SceneBehaviour
{
public:
    Burner(const std::string& name = "BurnerPbdObjectBehavior");

    void init() override;

    ///
    /// \brief Get the object doing the burning
    ///@{
    std::shared_ptr<PbdObject> getBurnerObject() const { return m_burningObj; }
    ///@}

    /// \brief Get/Set the geometry doing the burning, if it's not set
    /// the collision geoemtry of the burner object will be used
    /// @{
    std::shared_ptr<AnalyticalGeometry> getBurnerGeometry() const { return m_burnGeometry; }
    void setBurnerGeometry(std::shared_ptr<AnalyticalGeometry> geom)
    {
        CHECK(geom != nullptr);
        m_burnGeometry = geom;
    }

    /// @}

    ///
    /// \brief Get/Set the ontime from [0,1] where 1 is on fully and any value less than
    /// 1 is the percent of on time (e.g. 0.8 = 80% on time). This is used to represent
    /// the percent of damage that goes into cutting vs cauterizing
    ///@{
    double getOnTime() const { return m_onTime; }
    void setOnTime(const double onTime)
    {
        CHECK(onTime >= 0.0 && onTime <= 1.0) << "On time in the burning component must be between 0 and 1 \n";
        m_onTime = onTime;
    }

    ///@}

    ///
    /// \brief Get/Set the wattage. This value is generally between 50-80 watts for
    /// L-hook monopolar devices. The setter also sets the nomalized wattage, which
    /// is the input divided by 100.
    ///@{
    double getWattage() const { return m_wattage; }
    void setWattage(const double wattage)
    {
        CHECK(wattage >= 10.0 && wattage <= 200.0) << "Wattage is currently forced to be within 10 and 200 for the burning tool, set value within these bounds \n";
        m_wattage     = wattage;
        m_normWattage = wattage / 100.0;
    }

    ///@}

    ///
    /// \brief Start/Stop the burn by changing the onState flag
    ///@{
    void start() { m_onState = true; }
    void stop() { m_onState = false; }
    ///@}

    ///
    /// \brief Add burnable object to list of things that can be burned
    ///@{
    void addObject(std::shared_ptr<PbdObject> obj)
    {
        m_burnableObjects.push_back({ obj, nullptr });
    }

    ///@}

    /// Get state of burner (on/off ~ true/false)
    bool getState() const { return m_onState; }

    // Check state of mesh and delete cells that are burned away
    // void update(double dt);
    void visualUpdate(const double& dt) override;

protected:

    // The handle does the picking to choose which cells are burned.
    void handle();

    void handleBurnable(int burnableId);

    // Applies the burn using the burn model
    void applyBurn(int burnableId, int cellId);

    // Burner model for the monopolar tool (variables defined below)
    // Damage evolution:   damage^{n+1} = damage^{n} + onTime*q*normWattage*dt
    // Visual evolution:   visual^{n+1} = visual^{n} + (1-onTime)*q*normWattage*dt
    void monopolarToolModel(double& burnDmg, double& burnVis, double dt);

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    std::shared_ptr<PbdObject> m_burningObj;            ///< PbdObject doing the burning
    std::shared_ptr<AnalyticalGeometry> m_burnGeometry; ///< Geometry doing the burning

    std::shared_ptr<TaskNode> m_burningHandleNode;

    struct Burnable
    {
        std::shared_ptr<PbdObject> object;
        std::shared_ptr<CellPicker> picker;
    };

    std::vector<Burnable> m_burnableObjects; ///< Set of burnable objects, currently set by user during setup

    double m_onTime      = 0.5;              ///< On time for energy burning tool from [0,1].
    double m_wattage     = 50;               ///< Tool wattage
    double m_normWattage = 0.5;              ///< Tool wattage / 100
    double m_q = 4.0;                        ///< Fitting parameter that modifies how quickly tissue gets burned

    bool m_onState  = false;                 ///< flag if tool is currently on or not
    bool m_burnOnce = false;
private:
    double m_burnTime = 0;
};
} // namespace imstk