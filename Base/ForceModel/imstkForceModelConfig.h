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

#ifndef imstkForceModelConfig_h
#define imstkForceModelConfig_h

#include <string.h>

// g3log
#include "g3log/g3log.hpp"

// vega
#include "configFile.h"

namespace imstk
{
    // TODO: Move to appropriate place
    enum class ForceModelType
    {
        StVK,
        Corotational,
        Linear,
        Invertible,
        MassSpring,
        PBD,
        Rigid,
        Fluid,
        none
    };

    // TODO: Move to appropriate place
    enum class HyperElasticMaterialType
    {
        StVK,
        NeoHookean,
        MooneyRivlin,
        none
    };

///
/// \class ForceModelConfig
///
/// \brief Configure the force model from an external file
/// At present this class is based off of vega config file format
/// but will be standardized to fit all the force models including PBD
///
/// ------------------------------- ------------------------------------------------------
///     Option Name                                 Description
/// ------------------------------- ------------------------------------------------------
///     femMethod                    FEM method used
///                                     [StVK (default), Corotational, Linear, Invertible, none]
///     invertibleMaterial           Constitutive law used
///                                     [StVK (default), NeoHookean, MooneyRivlin, none]
///     fixedDOFFilename             List of fixed degrees of freedom
///                                     [path to file containing indices]
///     dampingMassCoefficient       Mass matrix scaling factor for damping matrix computation
///                                     [default = 0.1; C = dampingMassCoefficient*M +
///                                                         dampingStiffnessCoefficient*K]
///     dampingStiffnessCoefficient  Stiffness matrix factor for damping matrix
///                                  computation
///                                     [default = 0.01; C = dampingMassCoefficient*M +
///                                                          dampingStiffnessCoefficient*K]
///     dampingLaplacianCoefficient  Laplacian damping matrix factor.
///                                     [default = 0.0]
///     deformationCompliance        Compliance factor.
///                                     [default = 1.0]
///     gravity                      Gravity constant.
///                                     [default = -9.81]
///     compressionResistance       Compression resistance parameter for the invertible
///                                 methods
///                                     [default = 500.0]
///     inversionThreshold          Inversion threshold parameter for the invertible methods
///                                     [default = -infinity]
///     numberOfThreads             Number of threads spawned by the force model
///                                     [default = 0]
///
///   TODO: Convert this to input through JSON format
class ForceModelConfig
{
    // TODO: Do this in a better way
    struct customOptionsList
    {
        char femMethod[256];
        char invertibleMaterial[256];
        char fixedDOFFilename[256];
        double dampingMassCoefficient = 0.1;
        double dampingStiffnessCoefficient = 0.01;
        double dampingLaplacianCoefficient = 0.0;
        double deformationCompliance = 1.0;
        double gravity = -9.81;
        double compressionResistance = 500.0;
        double inversionThreshold = -std::numeric_limits< double >::max();
        int numberOfThreads = 0;
    };

    struct customOptionsNameList
    {
        char femMethodName[256] = "femMethod";
        char invertibleMaterialName[256] = "invertibleMaterial";
        char fixedDOFFilenameName[256] = "fixedDOFFilename";
        char dampingMassCoefficientName[256] = "dampingMassCoefficient";
        char dampingLaplacianCoefficientName[256] = "dampingLaplacianCoefficient";
        char dampingStiffnessCoefficientName[256] = "dampingStiffnessCoefficient";
        char deformationComplianceName[256] = "deformationCompliance";
        char compressionResistanceName[256] = "compressionResistance";
        char inversionThresholdName[256] = "inversionThreshold";
        char numberOfThreadsName[256] = "numberOfThreads";
        char gravityName[256] = "gravity";
    };
public:

    ///
    /// \brief Constructor
    ///
    ForceModelConfig(const std::string &configurationFileName);

    ///
    /// \brief Destructor
    ///
    ~ForceModelConfig(){};

    ///
    /// \brief Parse the deformable objeyc config file
    ///
    bool parseConfig(const std::string &configFileName);

    ///
    /// \brief Get the options set that are floating points types
    ///
    const std::map<std::string, double>& getFloatsOptionsMap()
    {
        return m_floatsOptionMap;
    }

    ///
    /// \brief Get the options set that are integer points types
    ///
    const std::map<std::string, int>& getIntegerOptionsMap()
    {
        return m_intsOptionMap;
    }

    ///
    /// \brief Get the options set that are string points types
    ///
    const std::map<std::string, std::string>& getStringOptionsMap()
    {
        return m_stringsOptionMap;
    }

    ///
    /// \brief Get the type of the force model
    ///
    ForceModelType getForceModelType();

    ///
    /// \brief Get the type of the hyperelastic model
    ///
    HyperElasticMaterialType getHyperelasticMaterialType();

protected:
    std::string m_vegaConfigFileName;                      ///> Store configuration file.
    std::map<std::string, double> m_floatsOptionMap;       ///> Map for floating point
    std::map<std::string, int> m_intsOptionMap;            ///> Map for int configuration variables
    std::map<std::string, std::string> m_stringsOptionMap; ///> Map for string

    bool m_loadSuccessful; ///> Configuration loading is successful
};

} // imstk

#endif // imstkForceModelConfig_h
