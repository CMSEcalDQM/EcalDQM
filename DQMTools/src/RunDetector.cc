// -*- C++ -*-
//
// Package:    RunDetector
// Class:      RunDetector
// 
/**\class RunDetector RunDetector.cc Toolset/RunDetector/src/RunDetector.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Yutaro Iiyama,512 1-005,+41227670489,
//         Created:  Fri Mar 16 13:46:49 CET 2012
// $Id: RunDetector.cc,v 1.4 2012/03/29 20:00:19 yiiyama Exp $
//
//


// system include files
#include <memory>
#include <fstream>
#include <sstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"

//
// class declaration
//

class RunDetector : public edm::EDAnalyzer {
public:
  explicit RunDetector(const edm::ParameterSet&);
  ~RunDetector() {}

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginJob() {}
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() {}

  virtual void beginRun(edm::Run const&, edm::EventSetup const&) {}
  virtual void endRun(edm::Run const&, edm::EventSetup const&) {}
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}

  // ----------member data ---------------------------

  edm::InputTag FEDRawDataCollectionTag_;

  std::string directory_;

  bool waitMode_;

  bool verbose_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
RunDetector::RunDetector(const edm::ParameterSet& iConfig) :
  FEDRawDataCollectionTag_(iConfig.getUntrackedParameter<edm::InputTag>("FEDRawDataCollectionTag")),
  directory_(iConfig.getUntrackedParameter<std::string>("directory")),
  waitMode_(iConfig.getUntrackedParameter<bool>("waitMode", false)),
  verbose_(iConfig.getUntrackedParameter<bool>("verbose", false))
{
  //now do what ever initialization is needed
}

//
// member functions
//

// ------------ method called when starting to processes a run  ------------
void 
RunDetector::analyze(edm::Event const& _event, edm::EventSetup const&)
{
  using namespace std;

  stringstream fileName;
  fileName << directory_ << "/" << _event.id().run();

  fstream indexFile;

  indexFile.open(fileName.str(), ios::in);
  if(indexFile.is_open()){
    if(verbose_)
      cout << "Found file " << fileName.str() << "; exiting." << endl;

    return;
  }
  else if(waitMode_)
    throw cms::Exception("RunNumberMismatch") << "Wait mode invoked for unindexed run!" << endl;

  indexFile.close();

  indexFile.open(fileName.str(), ios::out);
  if(!indexFile.is_open())
    throw cms::Exception("IOError") << "Cannot create file " << fileName.str() << endl;

  edm::Handle<FEDRawDataCollection> rawHndl;

  if(!_event.getByLabel(FEDRawDataCollectionTag_, rawHndl))
    throw cms::Exception("ProductNotFound") << "FEDRawDataCollection " << FEDRawDataCollectionTag_ << endl;

  bool ecalIn(false);
  for(int iFED(601); iFED <= 654; iFED++){
    if(rawHndl->FEDData(iFED).size() > 32){
      ecalIn = true;
      break;
    }
  }

  indexFile << (ecalIn ? "IN" : "OUT") << endl;

  indexFile.close();

  if(verbose_){
    if(ecalIn)
      cout << "Found ECAL data in run " << _event.id().run() << ". Start monitoring!" << endl;
    else
      cout << "Run " << _event.id().run() << " does not contain ECAL data." << endl;
  }
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
RunDetector::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(RunDetector);
