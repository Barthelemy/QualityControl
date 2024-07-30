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
/// \file   UserCodeInterface.cxx
/// \author Barthelemy von Haller
///

#include "QualityControl/UserCodeInterface.h"
#include <thread>
#include "QualityControl/QcInfoLogger.h"

using namespace o2::ccdb;

namespace o2::quality_control::core
{

void UserCodeInterface::setCustomParameters(const CustomParameters& parameters)
{
  mCustomParameters = parameters;
  configure();
}

const std::string& UserCodeInterface::getName() const {
  return mName;
}

void UserCodeInterface::setName(const std::string& name) {
  mName = name;
}

void UserCodeInterface::enableCtpScalers()
{
  // TODO bail if we are in async

  // TODO get the interval from config

  int intervalMinutes = 1;

  // call a first time
  updateScalers();

  // Start the periodic method call in a separate thread
  std::thread periodicThread(&UserCodeInterface::regularCallback, this, intervalMinutes);

  // Detach the thread so it runs independently
  periodicThread.detach();
}

void UserCodeInterface::regularCallback(int intervalMinutes) {
  while (true) {
    std::this_thread::sleep_for(std::chrono::minutes(intervalMinutes));
    updateScalers();
  }
}

void UserCodeInterface::updateScalers() {
  ILOG(Info, Devel) << " *-*-*-*-*- Update scalers " << ENDM;

  if(! mDatabase) {
    ILOG(Error, Devel) << " database not set ! " << ENDM;

    return;
    // todo handle the case when database is not set
  }

  std::map<std::string, std::string> meta;
  void* rawResult = mDatabase->retrieveAny(typeid(o2::ctp::CTPRunScalers), "qc/CTP/Scalers", meta); // TODO make sure we get the last one.
  mScalers = std::shared_ptr< o2::ctp::CTPRunScalers >(static_cast<o2::ctp::CTPRunScalers*>(rawResult));
}

using namespace std;

void UserCodeInterface::setDatabase(std::unordered_map<std::string, std::string> dbConfig)
{
  // TODO one could really argue that it would be easier to have a singleton for the QCDB... because here we will build and save a database object

  if(dbConfig.count("implementation") == 0 || dbConfig.count("host")) {
    ILOG(Error, Devel) << "dbConfig is incomplete, we don't build the user code database instance " << ENDM;
    return;
    // todo
  }

  cout << " SET DATABASE " << dbConfig.size() << endl;
  for (auto pair : dbConfig) {
    ILOG(Info,Devel) << pair.first << " : " << pair.second << ENDM;
  }

  // for every user code we instantiate.
  mDatabase = repository::DatabaseFactory::create(dbConfig.at("implementation"));
  mDatabase->connect(dbConfig);
  ILOG(Info, Devel) << "Database that is going to be used > Implementation : " << dbConfig.at("implementation") << " / Host : " << dbConfig.at("host") << ENDM;
}

std::shared_ptr<ctp::CTPRunScalers> UserCodeInterface::getScalers(long timestamp)
{
  return mScalers;
}

} // namespace o2::quality_control::core