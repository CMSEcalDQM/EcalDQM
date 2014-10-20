#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/EcalRawData/interface/EcalDCCHeaderBlock.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>

#include "TSystem.h"
#include "TFile.h"
#include "TString.h"

extern int opterr;
extern int optopt;
extern int optind;
extern char* optarg;

int
main(int argc, char** argv)
{
  TString collectionTag("rawDataCollector");

  bool parseOpts(true);
  int opt(0);
  while(parseOpts){
    opt = getopt(argc, argv, ":c:h");
    switch(opt){
    case '?':
      std::cerr << "Unknown option -" << char(optopt) << std::endl;
      break;
    case ':':
      std::cerr << "Option -" << char(optopt) << " requires an argument" << std::endl;
      parseOpts = false;
      break;
    case 'c':
      collectionTag = optarg;
      break;
    case 'h':
      std::cout << "Usage: ecalhdr [-c Collection] input" << std::endl;
      return 0;
    default:
      parseOpts = false;
      break;
    }
  }

  if(optind != argc - 1){
    std::cerr << "Usage: ecalhdr [-c Collection] input" << std::endl;
    return 1;
  }

  gSystem->Load("libFWCoreFWLite");
  AutoLibraryLoader::enable();

  TFile* input(TFile::Open(argv[optind]));
  fwlite::Event event(input);

  fwlite::Handle<FEDRawDataCollection> rawData;

  for(unsigned iFED(1); iFED != 55; ++iFED)
    std::cout << 600 + iFED << " ";
  std::cout << std::endl;

  std::stringstream ss;

  for(event.toBegin(); !event.atEnd(); ++event){
    ss.clear();
    ss.str("");

    rawData.getByLabel(event, collectionTag);

    if(!rawData.isValid()){
      std::cout << "FEDRawDataCollection " << collectionTag << " not found" << std::endl;
      break;
    }

    ss << "--------------------- " << event.id().event() << " ---------------------" << std::endl;

    bool hasEcal(false);

    for(unsigned iFED(601); iFED != 655; ++iFED){
      FEDRawData  const& fedData(rawData->FEDData(iFED));

      if(fedData.size() / sizeof(uint64_t) < 2){
	ss << " -  ";
	continue;
      }

      hasEcal = true;

      uint64_t const* pdata(reinterpret_cast<uint64_t const*>(fedData.data()));
      unsigned char trigType((*pdata >> 56) & 0xf);
      ++pdata;
      ++pdata;
      unsigned char type((*pdata >> 8) & 0x7);
      unsigned char sequence((*pdata >> 11) & 0x7);

      // taken from EventFilter/EcalRawToDigi/interface/EcalDCCHeaderRuntypeDecoder.cc

      int eventType(-1);

      if(trigType == 1){
	if(type ==0 && sequence == 0) eventType = EcalDCCHeaderBlock::COSMIC;
	else if(type ==0 && sequence == 1) eventType = EcalDCCHeaderBlock::COSMIC;
	else if(type ==0 && sequence == 2) eventType = EcalDCCHeaderBlock::BEAMH4;
	else if(type ==0 && sequence == 3) eventType = EcalDCCHeaderBlock::BEAMH2;
	else if(type ==0 && sequence == 4) eventType = EcalDCCHeaderBlock::MTCC;
	else if(type ==1 && sequence == 0) eventType = EcalDCCHeaderBlock::LASER_STD;
	else if(type ==1 && sequence == 1) eventType = EcalDCCHeaderBlock::LASER_POWER_SCAN;
	else if(type ==1 && sequence == 2) eventType = EcalDCCHeaderBlock::LASER_DELAY_SCAN;
	else if(type ==2 && sequence == 0) eventType = EcalDCCHeaderBlock::TESTPULSE_SCAN_MEM;
	else if(type ==2 && sequence == 1) eventType = EcalDCCHeaderBlock::TESTPULSE_MGPA;
	else if(type ==3 && sequence == 0) eventType = EcalDCCHeaderBlock::PEDESTAL_STD;
	else if(type ==3 && sequence == 1) eventType = EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN;
	else if(type ==3 && sequence == 2) eventType = EcalDCCHeaderBlock::PEDESTAL_25NS_SCAN;
	else if(type ==4 && sequence == 0) eventType = EcalDCCHeaderBlock::LED_STD;
	else if(type ==5 && sequence == 0) eventType = EcalDCCHeaderBlock::PHYSICS_GLOBAL;
	else if(type ==5 && sequence == 1) eventType = EcalDCCHeaderBlock::COSMICS_GLOBAL;
	else if(type ==5 && sequence == 2) eventType = EcalDCCHeaderBlock::HALO_GLOBAL;
	else if(type ==6 && sequence == 0) eventType = EcalDCCHeaderBlock::PHYSICS_LOCAL;
	else if(type ==6 && sequence == 1) eventType = EcalDCCHeaderBlock::COSMICS_LOCAL;
	else if(type ==6 && sequence == 2) eventType = EcalDCCHeaderBlock::HALO_LOCAL;
      }
      else if(trigType == 2){
	bool isLocal(type ==6 && sequence == 3);
	int detailedTriggerTypeInTTCCommand((*pdata >> 40) & 0x7);

	switch(detailedTriggerTypeInTTCCommand){
	case EcalDCCHeaderBlock::TTC_LASER:
	  if(isLocal) eventType = EcalDCCHeaderBlock::LASER_STD;
	  else eventType = EcalDCCHeaderBlock::LASER_GAP;
	  break;
	case EcalDCCHeaderBlock::TTC_LED:
	  if(isLocal) eventType = EcalDCCHeaderBlock::LED_STD;
	  else eventType = EcalDCCHeaderBlock::LED_GAP;
	  break;
	case EcalDCCHeaderBlock::TTC_TESTPULSE:
	  if(isLocal) eventType = EcalDCCHeaderBlock::TESTPULSE_MGPA;
	  else eventType = EcalDCCHeaderBlock::TESTPULSE_GAP;
	  break;
	case EcalDCCHeaderBlock::TTC_PEDESTAL:
	  if(isLocal) eventType = EcalDCCHeaderBlock::PEDESTAL_STD;
	  else eventType = EcalDCCHeaderBlock::PEDESTAL_GAP;
	  break;
	default:
	  break;
	}
      }

      char typeStr[10];
      std::sprintf(typeStr, " %d ", eventType);
      if(typeStr[2] == '\0'){
	typeStr[2] = ' ';
	typeStr[3] = '\0';
      }
      ss << typeStr;
    }

    if(hasEcal)
      std::cout << ss.str() << std::endl;
  }
      
  std::cout << EcalDCCHeaderBlock::COSMIC << " COSMIC" << "";
  std::cout << EcalDCCHeaderBlock::BEAMH4 << " BEAMH4" << " ";
  std::cout << EcalDCCHeaderBlock::BEAMH2 << " BEAMH2" << " ";
  std::cout << EcalDCCHeaderBlock::MTCC << " MTCC" << std::endl;
  std::cout << EcalDCCHeaderBlock::LASER_STD << " LASER_STD" << " ";
  std::cout << EcalDCCHeaderBlock::LASER_POWER_SCAN << " LASER_POWER_SCAN" << " ";
  std::cout << EcalDCCHeaderBlock::LASER_DELAY_SCAN << " LASER_DELAY_SCAN" << " ";
  std::cout << EcalDCCHeaderBlock::TESTPULSE_SCAN_MEM << " TESTPULSE_SCAN_MEM" << std::endl;
  std::cout << EcalDCCHeaderBlock::TESTPULSE_MGPA << " TESTPULSE_MGPA" << " ";
  std::cout << EcalDCCHeaderBlock::PEDESTAL_STD << " PEDESTAL_STD" << " ";
  std::cout << EcalDCCHeaderBlock::PEDESTAL_OFFSET_SCAN << " PEDESTAL_OFFSET_SCAN" << " ";
  std::cout << EcalDCCHeaderBlock::PEDESTAL_25NS_SCAN << " PEDESTAL_25NS_SCAN" << std::endl;
  std::cout << EcalDCCHeaderBlock::LED_STD << " LED_STD" << " ";
  std::cout << EcalDCCHeaderBlock::PHYSICS_GLOBAL << " PHYSICS_GLOBAL" << " ";
  std::cout << EcalDCCHeaderBlock::COSMICS_GLOBAL << " COSMICS_GLOBAL" << " ";
  std::cout << EcalDCCHeaderBlock::HALO_GLOBAL << " HALO_GLOBAL" << std::endl;
  std::cout << EcalDCCHeaderBlock::PHYSICS_LOCAL << " PHYSICS_LOCAL" << " ";
  std::cout << EcalDCCHeaderBlock::COSMICS_LOCAL << " COSMICS_LOCAL" << " ";
  std::cout << EcalDCCHeaderBlock::HALO_LOCAL << " HALO_LOCAL" << " ";
  std::cout << EcalDCCHeaderBlock::LASER_STD << " LASER_STD" << std::endl;
  std::cout << EcalDCCHeaderBlock::LASER_GAP << " LASER_GAP" << " ";
  std::cout << EcalDCCHeaderBlock::LED_STD << " LED_STD" << " ";
  std::cout << EcalDCCHeaderBlock::LED_GAP << " LED_GAP" << " ";
  std::cout << EcalDCCHeaderBlock::TESTPULSE_MGPA << " TESTPULSE_MGPA" << std::endl;
  std::cout << EcalDCCHeaderBlock::TESTPULSE_GAP << " TESTPULSE_GAP" << " ";
  std::cout << EcalDCCHeaderBlock::PEDESTAL_STD << " PEDESTAL_STD" << " ";
  std::cout << EcalDCCHeaderBlock::PEDESTAL_GAP << " PEDESTAL_GAP" << " ";      
  std::cout << std::endl;

  return 0;
}
