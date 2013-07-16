// -*- C++ -*-
//
// Package:    EcalRawDataReader
// Class:      EcalRawDataReader
// 
/**\class EcalRawDataReader EcalRawDataReader.cc Toolset/EcalRawDataReader/src/EcalRawDataReader.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Yutaro Iiyama,512 1-005,+41227670489,
//         Created:  Fri Mar 16 13:46:49 CET 2012
// $Id: EcalRawDataReader.cc,v 1.3 2012/03/25 19:55:50 yiiyama Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/Common/interface/Handle.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"

#include "DataFormats/EcalRawData/interface/EcalRawDataCollections.h"

#include "TFile.h"
#include "TH1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"

//
// class declaration
//

class EcalRawDataReader : public edm::EDAnalyzer {
public:
  explicit EcalRawDataReader(const edm::ParameterSet&);
  ~EcalRawDataReader();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  enum Plots {
    p_DCCSize,
    h_experimentType,
    h_runType,
    h_basicTrigger,
    //    h_detailedTrigger,
    h2_basicTriggerVrunType,
    //    h2_detailedTriggerVbasicTrigger,
    nPlots
  };

private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  virtual void beginRun(edm::Run const&, edm::EventSetup const&);
  virtual void endRun(edm::Run const&, edm::EventSetup const&);
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

  // ----------member data ---------------------------

  edm::InputTag FEDRawDataCollectionTag_;
  edm::InputTag EcalRawDataCollectionTag_;

  TFile* output_;
  TH1* plots_[nPlots];
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
EcalRawDataReader::EcalRawDataReader(const edm::ParameterSet& iConfig) :
  FEDRawDataCollectionTag_(iConfig.getParameter<edm::InputTag>("FEDRawDataCollectionTag")),
  EcalRawDataCollectionTag_(iConfig.getParameter<edm::InputTag>("EcalRawDataCollectionTag")),
  output_(0)
{
  //now do what ever initialization is needed

  std::string fileName(iConfig.getUntrackedParameter<std::string>("fileName"));

  output_ = new TFile(fileName.c_str(), "recreate");

  plots_[p_DCCSize] = new TH1F("p_DCCSize", "DCC data size", 54, 600.5, 654.5);

  plots_[h_experimentType] = new TH1F("h_experimentType", "Experiment Type", 16, -0.5, 15.5);

  plots_[h_runType] = new TH1F("h_runType", "Run Type", 24, -0.5, 23.5);
  plots_[h_basicTrigger] = new TH1F("h_basicTrigger", "Basic Trigger Type", 4, 0.5, 4.5);
  //  plots_[h_detailedTrigger] = new TH1F("h_detailedTrigger", "Detailed Trigger Type", 4, 3.5, 7.5);

  plots_[h2_basicTriggerVrunType] = new TH2F("h2_basicTriggerVrunType", "Basic Trigger v Run Type", 24, -0.5, 23.5, 4, 0.5, 4.5);
  //  plots_[h2_detailedTriggerVbasicTrigger] = new TH2F("h2_detailedTriggerVbasicTrigger", "Detailed Trigger v Basic Trigger", 4, 0.5, 4.5, 4, 3.5, 7.5);
}


EcalRawDataReader::~EcalRawDataReader()
{
  output_->cd(); 

  for(unsigned iPlot(0); iPlot < nPlots; iPlot++)
    plots_[iPlot]->Write();

  output_->Close();
  delete output_;
}


//
// member functions
//

// ------------ method called for each event  ------------
void
EcalRawDataReader::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  plots_[h_experimentType]->Fill(iEvent.experimentType());

  edm::Handle<FEDRawDataCollection> fedRaw;
  if(!iEvent.getByLabel(FEDRawDataCollectionTag_, fedRaw))
    throw cms::Exception("ProductNotFound");

  for(int iFED(601); iFED < 655; iFED++)
    plots_[p_DCCSize]->Fill(iFED, fedRaw->FEDData(iFED).size());

  edm::Handle<EcalRawDataCollection> dcchHndl;

  if(!iEvent.getByLabel(EcalRawDataCollectionTag_, dcchHndl))
    throw cms::Exception("ProductNotFound");

  for(EcalRawDataCollection::const_iterator dcchItr(dcchHndl->begin()); dcchItr != dcchHndl->end(); ++dcchItr){
    plots_[h_runType]->Fill(dcchItr->getRunType());
    plots_[h_basicTrigger]->Fill(dcchItr->getBasicTriggerType());
    plots_[h2_basicTriggerVrunType]->Fill(dcchItr->getRunType(), dcchItr->getBasicTriggerType());
  }
}


// ------------ method called once each job just before starting event loop  ------------
void 
EcalRawDataReader::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
EcalRawDataReader::endJob() 
{
}

// ------------ method called when starting to processes a run  ------------
void 
EcalRawDataReader::beginRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
EcalRawDataReader::endRun(edm::Run const&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
EcalRawDataReader::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
EcalRawDataReader::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
EcalRawDataReader::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(EcalRawDataReader);
