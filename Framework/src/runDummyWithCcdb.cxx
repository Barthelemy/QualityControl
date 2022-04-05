// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file    runBasic.cxx
/// \author  Piotr Konopka
///
/// \brief This is an executable showing QC Task's usage in Data Processing Layer.
///
/// This is an executable showing QC Task's usage in Data Processing Layer. The workflow consists of data producer,
/// which generates arrays of random size and content. Its output is dispatched to QC task using Data Sampling
/// infrastructure. QC Task runs exemplary user code located in SkeletonDPL. The checker performs a simple check of
/// the histogram shape and colorizes it. The resulting histogram contents are shown in logs by printer.
///
/// QC task and CheckRunner are instantiated by respectively TaskFactory and CheckRunnerFactory,
/// which use preinstalled config file, that can be found in
/// ${QUALITYCONTROL_ROOT}/etc/basic.json or Framework/basic.json (original one).
///
/// To launch it, build the project, load the environment and run the executable:
///   \code{.sh}
///   > aliBuild build QualityControl --defaults o2
///   > alienv enter QualityControl/latest
///   > o2-qc-run-basic
///   \endcode
/// If you have glfw installed, you should see a window with the workflow visualization and sub-windows for each Data
/// Processor where their logs can be seen. The processing will continue until the main window it is closed. Regardless
/// of glfw being installed or not, in the terminal all the logs will be shown as well.

#include <DataSampling/DataSampling.h>
#include "QualityControl/InfrastructureGenerator.h"
#include "QualityControl/DummySpec.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::utilities;

// The customize() functions are used to declare the executable arguments and to specify custom completion and channel
// configuration policies. They have to be above `#include "Framework/runDataProcessing.h"` - that header checks if
// these functions are defined by user and if so, it invokes them. It uses a trick with SFINAE expressions to do that.

void customize(std::vector<CompletionPolicy>& policies)
{
  DataSampling::CustomizeInfrastructure(policies);
  quality_control::customizeInfrastructure(policies);
}

void customize(std::vector<ChannelConfigurationPolicy>& policies)
{
  DataSampling::CustomizeInfrastructure(policies);
}

void customize(std::vector<ConfigParamSpec>& workflowOptions)
{
  workflowOptions.push_back(
    ConfigParamSpec{ "config-path", VariantType::String, "", { "Absolute path to the config file. Overwrite the default paths. Do not use with no-data-sampling." } });
  workflowOptions.push_back(
    ConfigParamSpec{ "no-producer", VariantType::Bool, false, { "Removes the producer." } });
}

#include <string>

#include <Framework/runDataProcessing.h>
#include <Configuration/ConfigurationFactory.h>
#include <Configuration/ConfigurationInterface.h>

#include "QualityControl/CheckRunner.h"
#include "QualityControl/InfrastructureGenerator.h"
#include "QualityControl/runnerUtils.h"
#include "QualityControl/ExamplePrinterSpec.h"
#include "QualityControl/DataProducer.h"
#include "QualityControl/TaskRunner.h"
#include "Framework/DataDescriptorQueryBuilder.h"

std::string getConfigPath(const ConfigContext& config);

using namespace o2;
using namespace o2::framework;
using namespace o2::configuration;
using namespace o2::quality_control::checker;
using namespace std::chrono;

WorkflowSpec defineDataProcessing(const ConfigContext& config)
{
  WorkflowSpec specs;

  QcInfoLogger::setFacility("runDummyWithCcdb");

  // The producer to generate some data in the workflow, needed if not ran by aliecs
  bool noProducer = config.options().get<bool>("no-producer");
  if(!noProducer) {
    DataProcessorSpec producer = getDataProducerSpec(1, 10000, 10);
    specs.push_back(producer);
  }

  // Path to the config file
  auto userConfigPath = config.options().get<std::string>("config-path");
  std::string qcConfigurationSource = std::string("json://") + userConfigPath ;
  ILOG(Info, Ops) << "Using config file '" << qcConfigurationSource << "'" << ENDM;

  // Generation of Data Sampling infrastructure
  auto configInterface = ConfigurationFactory::getConfiguration(qcConfigurationSource);
  auto dataSamplingTree = configInterface->getRecursive("dataSamplingPolicies");
  DataSampling::GenerateInfrastructure(specs, dataSamplingTree);

  Inputs inputs = DataDescriptorQueryBuilder::parse("dummy:DS/tst-raw0/0?lifetime=condition&ccdb-path=GRP/GRP/LHCData/1613572714972");
  DataProcessorSpec dummy{
    "dummy",
    inputs,
    Outputs{},
    adaptFromTask<o2::quality_control::example::DummySpec>()
  };
  specs.push_back(dummy);

  return specs;
}
