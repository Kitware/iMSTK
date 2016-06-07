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

#include "imstkForceModelConfig.h"

namespace imstk
{

ForceModelConfig::ForceModelConfig(const std::string &configurationFile, const bool verbose)
{
    if (configurationFile.empty())
    {
        LOG(INFO) << "WARNING: Empty configuration filename." << std::endl;
        return;
    }

    this->m_vegaConfigFile = configurationFile;

    vega::ConfigFile vegaConfigFileOptions;

    char femMethod[256];
    vegaConfigFileOptions.addOptionOptional("femMethod",
        femMethod,
        "StVK");

    char invertibleMaterial[256];
    vegaConfigFileOptions.addOptionOptional("invertibleMaterial",
        invertibleMaterial,
        "StVK");

    char fixedDOFFilename[256];
    vegaConfigFileOptions.addOptionOptional("fixedDOFFilename",
        fixedDOFFilename,
        "");

    double dampingMassCoefficient = 0.1;
    vegaConfigFileOptions.addOptionOptional("dampingMassCoefficient",
        &dampingMassCoefficient,
        dampingMassCoefficient);

    double dampingStiffnessCoefficient = 0.01;
    vegaConfigFileOptions.addOptionOptional("dampingStiffnessCoefficient",
        &dampingStiffnessCoefficient,
        dampingStiffnessCoefficient);

    double dampingLaplacianCoefficient = 0.0;
    vegaConfigFileOptions.addOptionOptional("dampingLaplacianCoefficient",
        &dampingLaplacianCoefficient,
        dampingLaplacianCoefficient);

    double deformationCompliance = 1.0;
    vegaConfigFileOptions.addOptionOptional("deformationCompliance",
        &deformationCompliance,
        deformationCompliance);

    double gravity = -9.81;
    vegaConfigFileOptions.addOptionOptional("gravity",
        &gravity,
        gravity);

    double compressionResistance = 500.0;
    vegaConfigFileOptions.addOptionOptional("compressionResistance",
        &compressionResistance,
        compressionResistance);

    double inversionThreshold = -std::numeric_limits< double >::max();
    vegaConfigFileOptions.addOptionOptional("inversionThreshold",
        &inversionThreshold,
        inversionThreshold);

    int numberOfThreads = 0;
    vegaConfigFileOptions.addOptionOptional("numberOfThreads",
        &numberOfThreads,
        numberOfThreads);

    // Parse the configuration file
    if (!configurationFile.empty() &&
        vegaConfigFileOptions.parseOptions(configurationFile.data()) != 0)
    {
        /// TODO: Log this.
    }

    // Print option variables
    if (verbose)
    {
        vegaConfigFileOptions.printOptions();
    }

    // Store parsed string values
    this->m_stringsOptionMap.emplace("femMethod", femMethod);
    this->m_stringsOptionMap.emplace("invertibleMaterial", invertibleMaterial);
    this->m_stringsOptionMap.emplace("fixedDOFFilename", fixedDOFFilename);

    // Store parsed floating point values
    this->m_floatsOptionMap.emplace("dampingMassCoefficient", dampingMassCoefficient);
    this->m_floatsOptionMap.emplace("dampingLaplacianCoefficient",
        dampingLaplacianCoefficient);
    this->m_floatsOptionMap.emplace("dampingStiffnessCoefficient",
        dampingStiffnessCoefficient);
    this->m_floatsOptionMap.emplace("deformationCompliance", deformationCompliance);
    this->m_floatsOptionMap.emplace("gravity", gravity);
    this->m_floatsOptionMap.emplace("compressionResistance", compressionResistance);
    this->m_floatsOptionMap.emplace("inversionThreshold", inversionThreshold);

    // Store parsed int values
    this->m_intsOptionMap.emplace("numberOfThreads", numberOfThreads);

    // Set up some variables
    if (this->m_stringsOptionMap["femMethod"] == "StVK")
    {
        this->m_forceModelType = ForceModelType::StVK;
    }
    else if (this->m_stringsOptionMap["femMethod"] == "Corotational")
    {
        this->m_forceModelType = ForceModelType::StVK;
    }
    else if (this->m_stringsOptionMap["femMethod"] == "Linear")
    {
        this->m_forceModelType = ForceModelType::StVK;
    }
    else if (this->m_stringsOptionMap["femMethod"] == "Invertible")
    {
        this->m_forceModelType = ForceModelType::StVK;
    }
    else
    {
        this->m_forceModelType = ForceModelType::none;
    }

    if (this->m_stringsOptionMap["invertibleMaterial"] == "StVK")
    {
        this->m_isotropicMaterialType = hyperElasticMaterialType::StVK;
    }
    else if (this->m_stringsOptionMap["invertibleMaterial"] == "NeoHookean")
    {
        this->m_isotropicMaterialType = hyperElasticMaterialType::NeoHookean;
    }
    else if (this->m_stringsOptionMap["invertibleMaterial"] == "MooneyRivlin")
    {
        this->m_isotropicMaterialType = hyperElasticMaterialType::MooneyRivlin;
    }
    else
    {
        this->m_isotropicMaterialType = hyperElasticMaterialType::none;
    }
};

} // imstk
