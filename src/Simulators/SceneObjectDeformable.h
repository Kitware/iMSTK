// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMVEGAFEMSCENEOBJECT_DEFORMABLE_H
#define SMVEGAFEMSCENEOBJECT_DEFORMABLE_H

// SimMedTK includes
#include "Mesh/VolumeMesh.h"
#include "Mesh/SurfaceMesh.h"
#include "Core/SceneObject.h"
#include "Core/ErrorLog.h"
#include "VegaObjectConfig.h"
#include "Mesh/VegaSceneObjectDeformable.h"

/// \brief Base class for any scene object that is defmormable
class smSceneObjectDeformable : public smSceneObject
{
public:

    /// \brief Constructor
    smSceneObjectDeformable();

    /// \brief Destructor
    ~smSceneObjectDeformable();

    /// \brief rest the object to inital configuration
    /// and reset initial states
    virtual void resetToInitialState() = 0;

    /// \brief Load specified meshes
    virtual void loadVolumeMesh() = 0;

    /// \brief Load the rendering mesh if it is designated
    virtual void loadSurfaceMesh() = 0;

    /// \brief Load the data related to the vertices that will be fixed
    virtual void loadFixedBC() = 0;

    /// \brief Forces as a result of user interaction
    /// (through an interface such as mouse or haptic device)
    /// with the scene during runtime are added here
    virtual void applyUserInteractionForces() = 0;

    /// \brief Use the computed displacemetnt
    /// update to interpolate to the secondary display mesh
    virtual void updateSecondaryRenderingMesh() = 0;

    /// \brief Update the deformations by time stepping
    virtual void advanceDynamics() = 0;

    /// \brief Append the contact forces (if any) to external forces
    void applyContactForces();

    /// \brief Set all contact forces to zero (if any)
    void setContactForcesToZero();

    /// \brief  Sets the contact force at a given location
    /// (not given node) in contact force vector
    void setContactForceOfNodeWithDofID(const int dofID, const smVec3d force);

    /// \brief  returns displacement of at a given location
    /// (not given node) in contact force vector
    smVec3d getDisplacementOfNodeWithDofID(const int dofID) const;

    /// \brief  returns velocity of at a given location
    /// (not given node) in contact force vector
    smVec3d getVelocityOfNodeWithDofID(const int dofID) const;

    /// \brief  returns acceleration of at a given location
    /// (not given node) in contact force vector
    smVec3d getAccelerationOfNodeWithDofID(const int dofID) const;

    /// \brief get the total number of nodes
    int getNumNodes() const;

    /// \brief get the number of fixed nodes
    int getNumFixedNodes() const;

    /// \brief get the total number of degree of freedom
    int getNumTotalDof() const;

    /// \brief get the total number of degree of freedom
    /// that are fixed
    int getNumFixedDof() const;

    /// \brief get the number of degree of freedom that are not fixed
    int getNumDof() const;

    /// \brief sets the objects specific render details
    /// Should be moved to base class in near future
    void setRenderDetail(const std::shared_ptr<smRenderDetail> &r);

    /// \brief enable drawing of secondary mesh (if initialized)
    void setRenderSecondaryMesh();

    /// \brief enable drawing of primary mesh
    void setRenderPrimaryMesh();

    /// \brief get the primary surface mesh
    std::shared_ptr<smSurfaceMesh> getPrimarySurfaceMesh() const;

    /// \brief get the secondary surface mesh
    std::shared_ptr<smSurfaceMesh> getSecondarySurfaceMesh() const;

    // Get generalized velocity vector
    std::vector<double> &getVelocities()
    {
        return this->uvel;
    }

    const std::vector<double> &getVelocities() const
    {
        return this->uvel;
    }

    // Get generalized forces vector
    std::vector<double> &getForces()
    {
        return this->f_ext;
    }

    const std::vector<double> &getForces() const
    {
        return this->f_ext;
    }

    // Get contact forces vector
    std::vector<double> &getContactForces()
    {
        return this->f_contact;
    }

    const std::vector<double> &getContactForces() const
    {
        return this->f_contact;
    }

protected:
    friend class smSceneObjectDeformableRenderDelegate;

    int numNodes;
    int numFixedNodes;
    int numTotalDOF;
    int numDOF;
    int numFixedDof;
    int timestepCounter;
    int subTimestepCounter;
    int pulledVertex; ///< vertex that is pulled by user using external force

    bool topologyAltered;
    bool renderSecondaryMesh;

    std::string ConfigFileName;

    // body states
    std::vector<double> u;          ///< displacement
    std::vector<double> uvel;       ///< derivative of displacement in time
    std::vector<double> uaccel;     ///< double derivative of displacement in time
    std::vector<double> f_ext;      ///< external forces
    std::vector<double> f_extBase;  ///< non-varying external forces
    std::vector<double> uSecondary; ///< interpolated displacement for secondary mesh
    std::vector<double> uInitial;   ///< initial displacement
    std::vector<double> velInitial; ///< initial velocity
    std::vector<double> f_contact;  ///< contact forces (if any)
    std::vector<double> forceLoads; ///< discrete external load inputs

    std::vector<int> fixedVertices; ///< fixed vertcies

    std::shared_ptr<smVolumeMesh> volumeMesh;
    std::shared_ptr<smSurfaceMesh> primarySurfaceMesh;
    std::shared_ptr<smSurfaceMesh> secondarySurfaceMesh;
};

#endif // SMVEGAFEMSCENEOBJECT_DEFORMABLE_H
