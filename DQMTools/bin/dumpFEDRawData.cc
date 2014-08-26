#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/FWLite/interface/Handle.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"

#include <iostream>
#include <iomanip>
#include <unistd.h>

#include "TSystem.h"
#include "TFile.h"
#include "TString.h"

extern int opterr;
extern int optopt;
extern int optind;
extern char* optarg;

void scanRaw(TFile*, TString const&, bool = false);
void dumpRaw(TFile*, TString const&, int, int, int);

int
main(int argc, char** argv)
{
  TString collectionTag("rawDataCollector");
  int iStart(0);
  int iFED(601);
  int nEvents(1);
  bool scan(false);
  bool all(false);

  opterr = 0;

  bool parseOpts(true);
  int opt(0);
  while(parseOpts){
    opt = getopt(argc, argv, ":c:s:F:n:SAh");
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
    case 's':
      iStart = TString(optarg).Atoi();
      break;
    case 'F':
      iFED = TString(optarg).Atoi();
      break;
    case 'n':
      nEvents = TString(optarg).Atoi();
      break;
    case 'S':
      scan = true;
      break;
    case 'A':
      all = true;
      break;
    case 'h':
      std::cout << "Usage: dumpFEDRawData [-c Collection][-s iStart][-F iFED][-n nEvents][-S] input" << std::endl;
      return 0;
    default:
      parseOpts = false;
      break;
    }
  }

  if(optind != argc - 1){
    std::cerr << "Usage: dumpFEDRawData [-c Collection][-s iStart][-F iFED][-n nEvents][-S] input" << std::endl;
    return 1;
  }

  gSystem->Load("libFWCoreFWLite");
  AutoLibraryLoader::enable();

  TFile* input(TFile::Open(argv[optind]));

  if(scan) scanRaw(input, collectionTag, all);
  else dumpRaw(input, collectionTag, iStart, iFED, nEvents);

  return 0;
}

void
scanRaw(TFile* input, TString const& collectionTag, bool all/* = false*/)
{
  fwlite::Event event(input);

  event.toBegin();

  fwlite::Handle<FEDRawDataCollection> rawData;
  rawData.getByLabel(event, collectionTag);

  if(!rawData.isValid()){
    std::cout << "FEDRawDataCollection " << collectionTag << " not found" << std::endl;
    return;
  }

  for(int iFED(all ? 0 : 601); iFED != (all ? FEDNumbering::lastFEDId() + 1 : 655); ++iFED)
    std::cout << iFED << " " << rawData->FEDData(iFED).size() << std::endl;
}

void
dumpRaw(TFile* input, TString const& collectionTag, int iStart, int iFED, int nEvents)
{
  fwlite::Event event(input);

  event.toBegin();
  int iEvent(0);
  while(!event.atEnd() && ++iEvent < iStart) ++event;
  if(event.atEnd()) return;

  nEvents += iEvent;

  while(iEvent++ != nEvents){
    fwlite::Handle<FEDRawDataCollection> rawData;
  
    rawData.getByLabel(event, collectionTag);
  
    if(!rawData.isValid()){
      std::cout << "FEDRawDataCollection " << collectionTag << " not found" << std::endl;
      return;
    }
  
    FEDRawData const* fedData(0);
  
    if(iFED < 0){
      bool found(false);
      iFED = 600;
      while(!found){
        fedData = &(rawData->FEDData(++iFED));
        if(fedData->size() > 0) found = true;
      }
    }
    else
      fedData = &(rawData->FEDData(iFED));
  
    uint64_t const * pdata = (uint64_t const *)fedData->data();
  
    std::cout << std::dec;
  
    std::cout << "--------- DUMP FED " << iFED << " EVENT " << iEvent << "-------------" << std::endl;
  
    std::cout << std::hex;
  
    for( unsigned u = 0; u < fedData->size()/(sizeof(uint64_t)/sizeof(unsigned char)); u++ )
      std::cout << std::setw(16) << std::setfill('0') << pdata[u] << std::endl;

    ++event;
  }

}
