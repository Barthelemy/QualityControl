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
/// \file   QcMFTClusterCheck.cxx
/// \author Tomas Herman
/// \author Guillermo Contreras
/// \author Katarina Krizkova Gajdosova
/// \author Diana Maria Krupova

// Fair
#include <fairlogger/Logger.h>
// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TLatex.h>
#include <TList.h>
// Quality Control
#include "MFT/QcMFTClusterCheck.h"
#include "QualityControl/MonitorObject.h"
#include "QualityControl/Quality.h"
#include "QualityControl/QcInfoLogger.h"

using namespace std;

namespace o2::quality_control_modules::mft
{

void QcMFTClusterCheck::configure()
{

  // this is how to get access to custom parameters defined in the config file at qc.tasks.<task_name>.taskParameters
  if (auto param = mCustomParameters.find("ZoneThresholdMedium"); param != mCustomParameters.end()) {
    ILOG(Info, Support) << "Custom parameter - ZoneThresholdMedium: " << param->second << ENDM;
    mZoneThresholdMedium = stoi(param->second);
  }
  if (auto param = mCustomParameters.find("ZoneThresholdBad"); param != mCustomParameters.end()) {
    ILOG(Info, Support) << "Custom parameter - ZoneThresholdBad: " << param->second << ENDM;
    mZoneThresholdBad = stoi(param->second);
  }
}

Quality QcMFTClusterCheck::check(std::map<std::string, std::shared_ptr<MonitorObject>>* moMap)
{
  Quality result = Quality::Null;

  for (auto& [moName, mo] : *moMap) {

    (void)moName;

    if (mo->getName() == "mClusterOccupancy") {
      auto* hChipOccupancy = dynamic_cast<TH1F*>(mo->getObject());

      float den = hChipOccupancy->GetBinContent(0); // normalisation stored in the uderflow bin

      for (int iBin = 0; iBin < hChipOccupancy->GetNbinsX(); iBin++) {
        if (hChipOccupancy->GetBinContent(iBin + 1) == 0) {
          hChipOccupancy->Fill(937); // number of chips with zero clusters stored in the overflow bin
        }
        float num = hChipOccupancy->GetBinContent(iBin + 1);
        float ratio = (den > 0) ? (num / den) : 0.0;
        hChipOccupancy->SetBinContent(iBin + 1, ratio);
      }
    }

    if (mo->getName() == "mClusterPatternIndex") {
      auto* hChipPattern = dynamic_cast<TH1F*>(mo->getObject());

      float den = hChipPattern->GetBinContent(0); // normalisation stored in the uderflow bin

      for (int iBin = 0; iBin < hChipPattern->GetNbinsX(); iBin++) {
        float num = hChipPattern->GetBinContent(iBin + 1);
        float ratio = (den > 0) ? (num / den) : 0.0;
        hChipPattern->SetBinContent(iBin + 1, ratio);
      }
    }

    if (mo->getName() == "mClusterSizeSummary") {
      auto* hClusterSizePixels = dynamic_cast<TH1F*>(mo->getObject());

      float den = hClusterSizePixels->GetBinContent(0); // normalisation stored in the uderflow bin

      for (int iBin = 0; iBin < hClusterSizePixels->GetNbinsX(); iBin++) {
        float num = hClusterSizePixels->GetBinContent(iBin + 1);
        float ratio = (den > 0) ? (num / den) : 0.0;
        hClusterSizePixels->SetBinContent(iBin + 1, ratio);
      }
    }

    if (mo->getName() == "mGroupedClusterSizeSummary") {
      auto* hGroupedClusterSizePixels = dynamic_cast<TH1F*>(mo->getObject());

      float den = hGroupedClusterSizePixels->GetBinContent(0); // normalisation stored in the uderflow bin

      for (int iBin = 0; iBin < hGroupedClusterSizePixels->GetNbinsX(); iBin++) {
        float num = hGroupedClusterSizePixels->GetBinContent(iBin + 1);
        float ratio = (den > 0) ? (num / den) : 0.0;
        hGroupedClusterSizePixels->SetBinContent(iBin + 1, ratio);
      }
    }

    if (mo->getName() == "mClusterOccupancySummary") {
      auto* histogram = dynamic_cast<TH2F*>(mo->getObject());

      float den = histogram->GetBinContent(0, 0); // normalisation stored in the uderflow bin
      float nEmptyBins = 0;                       // number of empty zones stored here

      for (int iBinX = 0; iBinX < histogram->GetNbinsX(); iBinX++) {
        for (int iBinY = 0; iBinY < histogram->GetNbinsY(); iBinY++) {
          float num = histogram->GetBinContent(iBinX + 1, iBinY + 1);
          float ratio = (den > 0) ? (num / den) : 0.0;
          histogram->SetBinContent(iBinX + 1, iBinY + 1, ratio);
          if ((histogram->GetBinContent(iBinX + 1, iBinY + 1)) == 0) {
            nEmptyBins = nEmptyBins + 1;
          }
        }
      }

      if (nEmptyBins <= mZoneThresholdMedium) {
        result = Quality::Good;
      }
      if (nEmptyBins > mZoneThresholdMedium && nEmptyBins <= mZoneThresholdBad) {
        result = Quality::Medium;
      }
      if (nEmptyBins > mZoneThresholdBad) {
        result = Quality::Bad;
      }
    }
  }
  return result;
}

std::string QcMFTClusterCheck::getAcceptedType() { return "TH1"; }

void QcMFTClusterCheck::beautify(std::shared_ptr<MonitorObject> mo, Quality checkResult)
{
  if (mo->getName().find("mClusterOccupancySummary") != std::string::npos) {
    auto* hOccupancySummary = dynamic_cast<TH2F*>(mo->getObject());

    if (checkResult == Quality::Good) {
      LOG(info) << "Quality::Good";
      TLatex* tl = new TLatex(0.15, 6.0, "Quality Good");
      tl->SetTextColor(kGreen);
      hOccupancySummary->GetListOfFunctions()->Add(tl);
      tl->Draw();
    } else if (checkResult == Quality::Medium) {
      LOG(info) << "Quality::Medium";
      TLatex* tl = new TLatex(0.15, 6.0, "Quality medium: notify the on-call by mail");
      tl->SetTextColor(kOrange);
      hOccupancySummary->GetListOfFunctions()->Add(tl);
      tl->Draw();
    } else if (checkResult == Quality::Bad) {
      LOG(info) << "Quality::Bad";
      TLatex* tl = new TLatex(0.15, 6.0, "Quality bad: call the on-call!");
      tl->SetTextColor(kRed);
      hOccupancySummary->GetListOfFunctions()->Add(tl);
      tl->Draw();
    }
  }
}

} // namespace o2::quality_control_modules::mft
