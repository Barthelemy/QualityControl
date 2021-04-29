// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// \file   RepositoryBenchmark.cxx
/// \author Barthelemy von Haller
///

#include "RepositoryBenchmark.h"

#include <chrono>
#include <thread> // this_thread::sleep_for
#include <Common/Exceptions.h>

#include <TH2F.h>
#include <TRandom.h>

#include <fairmq/FairMQLogger.h>

#include <Common/Exceptions.h>

#include "QualityControl/DatabaseFactory.h"
#include "QualityControl/QcInfoLogger.h"

using namespace std;
using namespace std::chrono;
using namespace AliceO2::Common;
using namespace o2::quality_control::repository;
using namespace o2::monitoring;

namespace o2::quality_control::core
{
using namespace AliceO2::Common;


TH1* RepositoryBenchmark::oldCreateHisto(uint64_t sizeObjects, string name) const
{
  TH1* myHisto;
  switch (sizeObjects) {
    case 1:
      myHisto = new TH1F(name.c_str(), "h", 100, 0, 99); // 3.6 kB
      break;
    case 10:
      myHisto = new TH1F(name.c_str(), "h", 10000, -9, 9); // 10kB
      break;
    case 100:
      myHisto = new TH2F(name.c_str(), "h", 260, 0, 99, 100, 0, 99); // 100kB
      break;
    case 500:
      myHisto = new TH2F(name.c_str(), "h", 1250, 0, 99, 100, 0, 99); // 500kB
      break;
    case 1000:
      myHisto = new TH2F(name.c_str(), "h", 2500, 0, 99, 100, 0, 99); // 1MB
      break;
    case 2500:
      myHisto = new TH2F(name.c_str(), "h", 6250, 0, 99, 100, 0, 99); // 2.5MB
      break;
    case 5000:
      myHisto = new TH2F(name.c_str(), "h", 12500, 0, 99, 100, 0, 99); // 5MB
      break;
    default:
      BOOST_THROW_EXCEPTION(
        FatalException() << errinfo_details(
          "size of histo must be 1, 10, 100, 500, 1000, 2500 or 5000 (was: " + to_string(mSizeObjects) + ")"));
  }
  return myHisto;
}

TH1* RepositoryBenchmark::newCreateHisto(uint64_t sizeObjects, string name)
{
  TH1* myHisto;
  int i, j;

  switch (sizeObjects) {
    case 1: // 4 kB
      i=1;
      j=1;
      break;
    case 10: // 10 kB
      i=10;
      j=90;
      break;
    case 100: // 100kB
      i=10;
      j=900;
      break;
    case 500: // 500 kB
      i=50;
      j=900;
      break;
    case 1000: // 1MB
      i=100;
      j=900;
      break;
    case 2500: // 2.5 MB
      i=250;
      j=900;
      break;
    case 5000: // 5 MB
      i=500;
      j=900;
      break;
    default:
      BOOST_THROW_EXCEPTION(
        FatalException() << errinfo_details(
          "size of histo must be 1, 10, 100, 500, 1000, 2500 or 5000 (was: " + to_string(mSizeObjects) + ")"));
  }

  myHisto = new TH2F(name.c_str(), "h", i, 0, i, j, 0, j);
  for (int first = 0 ; first < i ; first++) {
    for (int second = 0; second < j; second++) {
      ((TH2F*)myHisto)->Fill(first, second, gRandom->Gaus(0., 1000.));
    }
  }

  return myHisto;
}

TH1* RepositoryBenchmark::createHisto(uint64_t sizeObjects, string name, bool oldBehaviour)
{
  cout << "oldbehaviour : " << oldBehaviour << endl;
  if (oldBehaviour) {
    ILOG(Info, Support) << "Histograms with old sizes. " << ENDM;
    return oldCreateHisto(sizeObjects, name);
  } else {
    ILOG(Info, Support) << "Histograms with new sizes. " << ENDM;
    return newCreateHisto(sizeObjects, name);
  }
}

  void RepositoryBenchmark::InitTask()
{
  // parse arguments database
  auto dbUrl = fConfig->GetValue<string>("database-url");
  auto dbBackend = fConfig->GetValue<string>("database-backend");
  mTaskName = fConfig->GetValue<string>("task-name");
  try {
    mDatabase = o2::quality_control::repository::DatabaseFactory::create(dbBackend);
    mDatabase->connect(fConfig->GetValue<string>("database-url"), fConfig->GetValue<string>("database-name"),
                       fConfig->GetValue<string>("database-username"), fConfig->GetValue<string>("database-password"));
    mDatabase->prepareTaskDataContainer(mTaskName);
  } catch (boost::exception& exc) {
    string diagnostic = boost::current_exception_diagnostic_information();
    ILOG(Error, Support) << "Unexpected exception, diagnostic information follows:\n"
                         << diagnostic << ENDM;
    if (diagnostic == "No diagnostic information available.") {
      throw;
    }
  }

  // parse other arguments
  try {
  mMaxIterations = fConfig->GetValue<uint64_t>("max-iterations");
  mNumberObjects = fConfig->GetValue<uint64_t>("number-objects");
  mSizeObjects = fConfig->GetValue<uint64_t>("size-objects");
  mDeletionMode = static_cast<bool>(fConfig->GetValue<int>("delete"));
  mObjectName = fConfig->GetValue<string>("object-name");
  auto numberTasks = fConfig->GetValue<uint64_t>("number-tasks");
  bool objectsSizeOldBehaviour = fConfig->GetProperty<bool>("size-objects-old-behaviour", true);
  int nbSmallObjects = fConfig->GetProperty<int>("add-small-objects", 0);

  // monitoring
  mMonitoring = MonitoringFactory::Get(fConfig->GetValue<string>("monitoring-url"));
  mThreadedMonitoring = static_cast<bool>(fConfig->GetValue<int>("monitoring-threaded"));
  mThreadedMonitoringInterval = fConfig->GetValue<int>("monitoring-threaded-interval");
  mMonitoring->enableProcessMonitoring(1); // collect every seconds metrics for this process
  mMonitoring->addGlobalTag("taskName", mTaskName);
  mMonitoring->addGlobalTag("numberObject", to_string(mNumberObjects));
  mMonitoring->addGlobalTag("sizeObject", to_string(mSizeObjects));
  if (mTaskName == "benchmarkTask_0") { // send these parameters to monitoring only once per benchmark run
    mMonitoring->send(Metric{ "ccdb_benchmark" }
                        .addValue(mNumberObjects, "number_objects")
                        .addValue(mSizeObjects * 1000, "size_objects")
                        .addValue(numberTasks, "number_tasks"));
  }

  if (mDeletionMode) {
    QcInfoLogger::GetInstance() << "Deletion mode..." << infologger::endm;
    emptyDatabase();
  }

  // prepare objects
  for (uint64_t i = 0; i < mNumberObjects; i++) {
    TH1* histo = createHisto(mSizeObjects, mObjectName + to_string(i), objectsSizeOldBehaviour);
    shared_ptr<MonitorObject> mo = make_shared<MonitorObject>(histo, mTaskName, "BMK");
    mo->setIsOwner(true);
    mMyObjects.push_back(mo);
  }
  for (uint64_t i = 0 ; i < nbSmallObjects ; i++) {
    TH1* histo = createHisto(mSizeObjects, mObjectName + "_small_" + to_string(i), objectsSizeOldBehaviour);
    shared_ptr<MonitorObject> mo = make_shared<MonitorObject>(histo, mTaskName, "BMK");
    mo->setIsOwner(true);
    mMyObjects.push_back(mo);
  }

  // start a timer in a thread to send monitoring metrics, if needed
  if (mThreadedMonitoring) {
    mTimer = new boost::asio::deadline_timer(io, boost::posix_time::seconds(mThreadedMonitoringInterval));
    mTimer->async_wait(boost::bind(&RepositoryBenchmark::checkTimedOut, this));
    th = new thread([&] { io.run(); });
  }
  } catch (...) {
    // catch the configuration exception and print it to avoid losing it
    ILOG(Fatal, Ops) << "Unexpected exception during configuration:\n"
                     << boost::current_exception_diagnostic_information(true) << ENDM;
    throw;
  }

  ILOG_INST.filterDiscardDebug(true);
}

void RepositoryBenchmark::checkTimedOut()
{
  mMonitoring->send({ mTotalNumberObjects, "ccdb_benchmark_objects_sent" }, DerivedMetricMode::RATE);

  // restart timer
  mTimer->expires_at(mTimer->expires_at() + boost::posix_time::seconds(mThreadedMonitoringInterval));
  mTimer->async_wait(boost::bind(&RepositoryBenchmark::checkTimedOut, this));
}

bool RepositoryBenchmark::ConditionalRun()
{
  if (mDeletionMode) { // the only way to not run is to return false from here.
    return false;
  }

  high_resolution_clock::time_point t1 = high_resolution_clock::now();

  // Store the object
  for (auto & mMyObject : mMyObjects) {
    mDatabase->storeMO(mMyObject);
    mTotalNumberObjects++;
  }
  if (!mThreadedMonitoring) {
    mMonitoring->send({ mTotalNumberObjects, "ccdb_benchmark_objects_sent" }, DerivedMetricMode::RATE);
  }

  high_resolution_clock::time_point t2 = high_resolution_clock::now();
  long duration = duration_cast<milliseconds>(t2 - t1).count();
  mMonitoring->send({ duration / (uint64_t)mMyObjects.size(), "ccdb_benchmark_store_duration_for_one_object_ms" });

  // determine how long we should wait till next iteration in order to have 1 sec between storage
  auto duration2 = duration_cast<microseconds>(t2 - t1);
  auto remaining = duration_cast<microseconds>(std::chrono::seconds(1) - duration2);
  //  QcInfoLogger::GetInstance() << "Remaining duration : " << remaining.count() << " us" << infologger::endm;
  if (remaining.count() < 0) {
    QcInfoLogger::GetInstance() << "Remaining duration is negative, we don't sleep " << infologger::endm;
  } else {
    this_thread::sleep_for(chrono::microseconds(remaining));
  }

  if (mMaxIterations > 0 && ++mNumIterations >= mMaxIterations) {
    QcInfoLogger::GetInstance() << "Configured maximum number of iterations reached. Leaving RUNNING state."
                                << infologger::endm;
    return false;
  }

  return true;
}

void RepositoryBenchmark::emptyDatabase()
{
  string prefix = "qc/BMK/MO/";
  mDatabase->truncate(mTaskName, mObjectName + "*");
}

} // namespace o2::quality_control::core
