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
/// \file   AggregatorInterface.h
/// \author Barthelemy von Haller
///

#ifndef QC_CHECKER_AGGREGATORINTERFACE_H
#define QC_CHECKER_AGGREGATORINTERFACE_H

#include <unordered_map>
#include <map>

#include "QualityControl/QualityObject.h"
#include "QualityControl/CommonInterface.h"
#include "QualityControl/Quality.h"

namespace o2::quality_control::checker
{

/// \brief  Skeleton of a quality aggregator user algorithm.
///
/// \author Barthelemy von Haller
class AggregatorInterface : public o2::quality_control::core::CommonInterface
{
 public:
  /// Default constructor
  AggregatorInterface() = default;
  /// Destructor
  virtual ~AggregatorInterface() = default;

  /// \brief Returns new qualities (usually fewer) based on the input qualities
  ///
  /// @param qoMap A map of the the QualityObjects to aggregate and their full names.
  /// @return The new qualities, associated with a name.
  virtual std::map<std::string, o2::quality_control::core::Quality> aggregate(std::map<std::string, std::shared_ptr<const o2::quality_control::core::QualityObject>>& qoMap) = 0;

 protected:
  ClassDef(AggregatorInterface, 2)
};

} // namespace o2::quality_control::checker

#endif /* QC_CHECKER_AGGREGATORINTERFACE_H */
