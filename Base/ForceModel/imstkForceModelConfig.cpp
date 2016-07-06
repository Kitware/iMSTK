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

ForceModelConfig::ForceModelConfig(const std::string &configFileName) : m_loadSuccessful(false)
{
    if (configFileName.empty())
    {
        LOG(INFO) << "WARNING: Empty configuration filename." << std::endl;
        return;
    }
    else
    {
        parseConfig(configFileName);
    }
};

bool
ForceModelConfig::parseConfig(const std::string &configFileName)
{
    vega::ConfigFile vegaConfigFileOptions;
    ForceModelConfig::customOptionsList optList;
    ForceModelConfig::customOptionsNameList optNameList;

    vegaConfigFileOptions.addOptionOptional(optNameList.femMethodName.c_str(), optList.femMethod, "StVK");
    vegaConfigFileOptions.addOptionOptional(optNameList.invertibleMaterialName.c_str(), optList.invertibleMaterial, "StVK");
    vegaConfigFileOptions.addOptionOptional(optNameList.fixedDOFFilenameName.c_str(), optList.fixedDOFFilename, "");
    vegaConfigFileOptions.addOptionOptional(optNameList.dampingMassCoefficientName.c_str(), &optList.dampingMassCoefficient, optList.dampingMassCoefficient);
    vegaConfigFileOptions.addOptionOptional(optNameList.dampingStiffnessCoefficientName.c_str(), &optList.dampingStiffnessCoefficient, optList.dampingStiffnessCoefficient);
    vegaConfigFileOptions.addOptionOptional(optNameList.dampingLaplacianCoefficientName.c_str(), &optList.dampingLaplacianCoefficient, optList.dampingLaplacianCoefficient);
    vegaConfigFileOptions.addOptionOptional(optNameList.deformationComplianceName.c_str(), &optList.deformationCompliance, optList.deformationCompliance);
    vegaConfigFileOptions.addOptionOptional(optNameList.gravityName.c_str(), &optList.gravity, optList.gravity);
    vegaConfigFileOptions.addOptionOptional(optNameList.compressionResistanceName.c_str(), &optList.compressionResistance, optList.compressionResistance);
    vegaConfigFileOptions.addOptionOptional(optNameList.inversionThresholdName.c_str(), &optList.inversionThreshold, optList.inversionThreshold);
    vegaConfigFileOptions.addOptionOptional(optNameList.numberOfThreadsName.c_str(), &optList.numberOfThreads, optList.numberOfThreads);


    // Parse the configuration file
    if (vegaConfigFileOptions.parseOptions(configFileName.data()) != 0)
    {
        this->m_vegaConfigFileName = configFileName;
        m_loadSuccessful = true;

        // Print option variables
        vegaConfigFileOptions.printOptions();
    }
    else
    {
        return false;
    }

    // Store parsed string values
    this->m_stringsOptionMap.emplace(optNameList.femMethodName, optList.femMethod);
    this->m_stringsOptionMap.emplace(optNameList.invertibleMaterialName, optList.invertibleMaterial);
    this->m_stringsOptionMap.emplace(optNameList.fixedDOFFilenameName, optList.fixedDOFFilename);

    // Store parsed floating point values
    this->m_floatsOptionMap.emplace(optNameList.dampingMassCoefficientName, optList.dampingMassCoefficient);
    this->m_floatsOptionMap.emplace(optNameList.dampingLaplacianCoefficientName, optList.dampingLaplacianCoefficient);
    this->m_floatsOptionMap.emplace(optNameList.dampingStiffnessCoefficientName, optList.dampingStiffnessCoefficient);
    this->m_floatsOptionMap.emplace(optNameList.deformationComplianceName, optList.deformationCompliance);
    this->m_floatsOptionMap.emplace(optNameList.gravityName, optList.gravity);
    this->m_floatsOptionMap.emplace(optNameList.compressionResistanceName, optList.compressionResistance);
    this->m_floatsOptionMap.emplace(optNameList.inversionThresholdName, optList.inversionThreshold);

    // Store parsed int values
    this->m_intsOptionMap.emplace(optNameList.numberOfThreadsName, optList.numberOfThreads);

    return true;
}

ForceModelType
ForceModelConfig::getForceModelType()
{
    // Set up some variables
    if (this->m_stringsOptionMap["femMethod"] == "StVK")
    {
        return ForceModelType::StVK;
    }
    else if (this->m_stringsOptionMap["femMethod"] == "Corotational")
    {
        return ForceModelType::Corotational;
    }
    else if (this->m_stringsOptionMap["femMethod"] == "Linear")
    {
        return ForceModelType::Linear;
    }
    else if (this->m_stringsOptionMap["femMethod"] == "Invertible")
    {
        return ForceModelType::Invertible;
    }
    else
    {
        LOG(INFO) << "Force model type not assigned";
        return ForceModelType::none;
    }
}

HyperElasticMaterialType
ForceModelConfig::getHyperelasticMaterialType()
{
    // Set up some variables
    if (this->m_stringsOptionMap["invertibleMaterial"] == "StVK")
    {
        return HyperElasticMaterialType::StVK;
    }
    else if (this->m_stringsOptionMap["invertibleMaterial"] == "NeoHookean")
    {
        return HyperElasticMaterialType::NeoHookean;
    }
    else if (this->m_stringsOptionMap["invertibleMaterial"] == "MooneyRivlin")
    {
        return HyperElasticMaterialType::MooneyRivlin;
    }
    else
    {
        LOG(INFO) << "Force model type not assigned";
        return HyperElasticMaterialType::none;
    }
}

} // imstk
