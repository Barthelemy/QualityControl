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
/// \file   DummySpec.h
/// \author Barthelemy von Haller
///

#ifndef QUALITYCONTROL_DUMMYSPEC_H
#define QUALITYCONTROL_DUMMYSPEC_H

#include <string>
#include <memory>

#include <TH1F.h>
#include <TObjArray.h>

#include <Framework/Task.h>
#include <Framework/DataRefUtils.h>

#include "QualityControl/MonitorObject.h"
#include "QualityControl/QualityObject.h"

namespace o2::quality_control::example
{

/**
 * \brief Dummy, empty, Spec to be used in tests.
 *
 */
class ExamplePrinterSpec : public framework::Task
{
 public:
  void run(ProcessingContext& processingContext) final
  {
    LOG(info) << "Received data";
  }
};

} // namespace o2::quality_control::example

#endif //QUALITYCONTROL_DUMMYSPEC_H
