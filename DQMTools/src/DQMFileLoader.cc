// -*- C++ -*-
//
// Package:    DQMFileLoader
// Class:      DQMFileLoader
// 
/**\class DQMFileLoader DQMFileLoader.cc Toolset/DQMFileLoader/src/DQMFileLoader.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Yutaro Iiyama,512 1-005,+41227670489,
//         Created:  Fri Mar 16 13:46:49 CET 2012
// $Id: DQMFileLoader.cc,v 1.5 2012/09/26 18:18:48 yiiyama Exp $
//
//


// system include files
#include <memory>
#include <dirent.h>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQMServices/Core/interface/DQMStore.h"

#include "TSystem.h"
#include "TString.h"
#include "TPRegexp.h"

//
// class declaration
//

class DQMFileLoader : public edm::EDAnalyzer {
public:
  explicit DQMFileLoader(const edm::ParameterSet&);
  ~DQMFileLoader() {}

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginJob() {}
  virtual void analyze(const edm::Event&, const edm::EventSetup&) {}
  virtual void endJob() {}

  virtual void beginRun(edm::Run const&, edm::EventSetup const&);
  virtual void endRun(edm::Run const&, edm::EventSetup const&) {}
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}

  // ----------member data ---------------------------

  std::string directory_;

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
DQMFileLoader::DQMFileLoader(const edm::ParameterSet& iConfig) :
  directory_(iConfig.getUntrackedParameter<std::string>("directory")),
  verbose_(iConfig.getUntrackedParameter<bool>("verbose", false))
{
  //now do what ever initialization is needed
}

//
// member functions
//

// ------------ method called when starting to processes a run  ------------
void 
DQMFileLoader::beginRun(edm::Run const& _run, edm::EventSetup const&)
{
  TString run(TString::Format("%09d", _run.run()));

  DQMStore* dqmStore(&(*(edm::Service<DQMStore>())));

  void* dir(gSystem->OpenDirectory(directory_.c_str()));
  TString fullpath((directory_ + "/").c_str());
  TString fileName;
  while((fileName = TString(gSystem->GetDirEntry(dir))) != ""){
    if(!fileName.Contains(run)) continue;
    if(!dqmStore->open((fullpath + fileName).Data(), false, "", "", DQMStore::StripRunDirs, false))
      throw cms::Exception("IOError") << fileName << " could not be processed by DQMStore";
  }
  gSystem->FreeDirectory(dir);
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
DQMFileLoader::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DQMFileLoader);
