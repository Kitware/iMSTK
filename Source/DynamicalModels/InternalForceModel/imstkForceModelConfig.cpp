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
#include <iostream>

namespace imstk
{
ForceModelConfig::ForceModelConfig(const std::string& configFileName) : m_loadSuccessful(false)
{
    if (configFileName.empty())
    {
        LOG(INFO) << "WARNING: Empty configuration filename.";
        return;
    }
    else
    {
        this->parseConfig(configFileName);
    }
};

bool
ForceModelConfig::parseConfig(const std::string& configFileName)
{
    vega::ConfigFile                        vegaConfigFileOptions;
    ForceModelConfig::customOptionsList     optList;
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
        LOG(FATAL) << "ForceModelConfig::parseConfig - Unable to load the configuration file";
        return false;
    }
    else
    {
        this->m_vegaConfigFileName = configFileName;
        m_loadSuccessful           = true;

        // Print option variables
        vegaConfigFileOptions.printOptions();
    }

    // get the root directory
    std::string  rootDir;
    const size_t last_slash_idx = configFileName.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        rootDir = configFileName.substr(0, last_slash_idx);
    }

    // Store parsed string values
    m_stringsOptionMap.emplace(optNameList.femMethodName, optList.femMethod);
    m_stringsOptionMap.emplace(optNameList.invertibleMaterialName, optList.invertibleMaterial);
    m_stringsOptionMap.emplace(optNameList.fixedDOFFilenameName, rootDir + std::string("/") + optList.fixedDOFFilename);

    // Store parsed floating point values
    m_floatsOptionMap.emplace(optNameList.dampingMassCoefficientName, optList.dampingMassCoefficient);
    m_floatsOptionMap.emplace(optNameList.dampingLaplacianCoefficientName, optList.dampingLaplacianCoefficient);
    m_floatsOptionMap.emplace(optNameList.dampingStiffnessCoefficientName, optList.dampingStiffnessCoefficient);
    m_floatsOptionMap.emplace(optNameList.deformationComplianceName, optList.deformationCompliance);
    m_floatsOptionMap.emplace(optNameList.gravityName, optList.gravity);
    m_floatsOptionMap.emplace(optNameList.compressionResistanceName, optList.compressionResistance);
    m_floatsOptionMap.emplace(optNameList.inversionThresholdName, optList.inversionThreshold);

    // Store parsed int values
    m_intsOptionMap.emplace(optNameList.numberOfThreadsName, optList.numberOfThreads);

    return true;
}

ForceModelType
ForceModelConfig::getForceModelType()
{
    // Set up some variables
    if (m_stringsOptionMap["femMethod"] == "StVK")
    {
        return ForceModelType::StVK;
    }
    else if (m_stringsOptionMap["femMethod"] == "CLFEM")
    {
        return ForceModelType::Corotational;
    }
    else if (m_stringsOptionMap["femMethod"] == "Linear")
    {
        return ForceModelType::Linear;
    }
    else if (m_stringsOptionMap["femMethod"] == "InvertibleFEM")
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
    if (m_stringsOptionMap["invertibleMaterial"] == "StVK")
    {
        return HyperElasticMaterialType::StVK;
    }
    else if (m_stringsOptionMap["invertibleMaterial"] == "NeoHookean")
    {
        return HyperElasticMaterialType::NeoHookean;
    }
    else if (m_stringsOptionMap["invertibleMaterial"] == "MooneyRivlin")
    {
        return HyperElasticMaterialType::MooneyRivlin;
    }
    else
    {
        LOG(INFO) << "Force model type not assigned";
        return HyperElasticMaterialType::none;
    }
}

void
ForceModelConfig::print()
{
    LOG(INFO) << "Floating point type options:\n";
    for (auto const& fo : m_floatsOptionMap)
    {
        LOG(INFO) << fo.first << ": " << fo.second;
    }
}
} // imstk
