// -*- C++ -*-
//
// Package:    FESyncAnalyzer
// Class:      FESyncAnalyzer
// 
/**\class FESyncAnalyzer FESyncAnalyzer.cc Toolset/FESyncAnalyzer/src/FESyncAnalyzer.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Yutaro Iiyama,512 1-005,+41227670489,
//         Created:  Fri Mar 16 13:46:49 CET 2012
// $Id: FESyncAnalyzer.cc,v 1.1 2012/06/26 11:07:39 yiiyama Exp $
//
//


// system include files
#include <memory>
#include <iomanip>
#include <fstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/EcalRawData/interface/EcalRawDataCollections.h"

#include "TString.h"
#include "TPRegexp.h"

//
// class declaration
//

class FESyncAnalyzer : public edm::EDFilter {
public:
  explicit FESyncAnalyzer(const edm::ParameterSet&);
  ~FESyncAnalyzer();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual bool filter(edm::Event&, const edm::EventSetup&);

  // ----------member data ---------------------------

  edm::InputTag fedRawTag_;
  edm::InputTag ecalRawDataTag_;
  std::ofstream output_;
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
FESyncAnalyzer::FESyncAnalyzer(const edm::ParameterSet& iConfig) :
  fedRawTag_(iConfig.getUntrackedParameter<edm::InputTag>("fedRawTag")),
  ecalRawDataTag_(iConfig.getUntrackedParameter<edm::InputTag>("ecalRawDataTag")),
  output_(iConfig.getUntrackedParameter<std::string>("outputFileName"))
{
  //now do what ever initialization is needed

  if(!output_.is_open())
    throw cms::Exception("IOError");
}

FESyncAnalyzer::~FESyncAnalyzer()
{
  output_.close();
}

//
// member functions
//

// ------------ method called when starting to processes a run  ------------
bool 
FESyncAnalyzer::filter(edm::Event& _evt, edm::EventSetup const&)
{
  edm::Handle<FEDRawDataCollection> fedHndl;

  edm::Handle<EcalRawDataCollection> rawHndl;

  if(!_evt.getByLabel(fedRawTag_, fedHndl)) return false;

  if(!_evt.getByLabel(ecalRawDataTag_, rawHndl)) return false;

  bool hasError(false);

  for(EcalRawDataCollection::const_iterator rawItr(rawHndl->begin()); rawItr != rawHndl->end(); ++rawItr){

    int ECALDCC_L1A(rawItr->getLV1());

    int ECALDCC_L1A_12bit(ECALDCC_L1A & 0xfff);

    const std::vector<short>& feLv1(rawItr->getFELv1());
    const std::vector<short>& status(rawItr->getFEStatus());

    for(int fe=0; fe<(int)feLv1.size(); fe++){

      if(status[fe] == 9 || status[fe] == 11) continue;

      if(feLv1[fe] + 1 != ECALDCC_L1A_12bit && feLv1[fe] != -1 && ECALDCC_L1A_12bit - 1 != -1){

	FEDRawData const& fedRaw(fedHndl->FEDData(rawItr->fedId()));

	uint64_t const* data(reinterpret_cast<uint64_t const*>(fedRaw.data()));

	output_ << std::dec;

	output_ << "=============Event " << _evt.id().event() << " FED " << rawItr->id() << "=========" << std::endl;

	output_ << std::hex;

	unsigned nWords(fedRaw.size() / (sizeof(uint64_t) / sizeof(unsigned char)));

	for(unsigned iWord(0); iWord < nWords; iWord++)
	  output_ << std::setw(16) << std::setfill('0') << data[iWord] << std::endl;

	output_ << std::endl;

	hasError = true;
	break;
      }
    }

    if(hasError) break;
  }

  return hasError;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
FESyncAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(FESyncAnalyzer);
