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
    enum class hyperElasticMaterialType
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
class ForceModelConfig
{
public:

    ///
    /// \brief Constructor
    ///
    ForceModelConfig(const std::string &configurationFile, const bool verbose = false);

    ~ForceModelConfig(){};

    const std::map<std::string, double>& getFloatsOptionsMap()
    {
        return m_floatsOptionMap;
    }

    const std::map<std::string, int>& getIntsOptionsMap()
    {
        return m_intsOptionMap;
    }

    const std::map<std::string, std::string>& getStringsOptionsMap()
    {
        return m_stringsOptionMap;
    }

protected:
    std::string m_vegaConfigFile;                          ///> Store configuration file.
    std::map<std::string, double> m_floatsOptionMap;       ///> Map for floating point
    std::map<std::string, int> m_intsOptionMap;            ///> Map for int configuration variables
    std::map<std::string, std::string> m_stringsOptionMap; ///> Map for string

    ForceModelType m_forceModelType;                  ///> Force model type used
    hyperElasticMaterialType m_isotropicMaterialType; ///> Hyperelastic constitutive law model
};

} // imstk

#endif // imstkForceModelConfig_h
