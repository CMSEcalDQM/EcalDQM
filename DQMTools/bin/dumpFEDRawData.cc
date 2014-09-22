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

void scanRaw(TFile*, TString const&, int, bool = false);
void dumpRaw(TFile*, TString const&, int, int, int);

class Word {
public:
  Word(uint64_t _data) : data_(_data) {}
  uint64_t data() const { return data_; }
  int operator()(unsigned _shift, unsigned _masklen) const
  {
    unsigned mask(0);
    for(unsigned i(0); i != _masklen; ++i) mask |= (1 << i);
    return ((data_ >> _shift) & mask);
  }

private:
  uint64_t data_;
};

std::ostream& operator<<(std::ostream& _stream, Word _w) { return _stream << _w.data(); }

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

  if(scan) scanRaw(input, collectionTag, iStart, all);
  else dumpRaw(input, collectionTag, iStart, iFED, nEvents);

  return 0;
}

void
scanRaw(TFile* input, TString const& collectionTag, int iStart, bool all/* = false*/)
{
  fwlite::Event event(input);

  event.toBegin();
  int iEvent(0);
  while(!event.atEnd() && ++iEvent < iStart) ++event;
  if(event.atEnd()) return;

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
  
    unsigned iData(0);
    unsigned blockSize(0);
    unsigned towerId(0);
    for(unsigned u(0); u < fedData->size()/(sizeof(uint64_t)/sizeof(unsigned char)); ++u){
      Word w(pdata[u]);
      std::cout << std::setw(16) << std::setfill('0') << w;
      if(w(62, 2) == 3){ // data block
        if(iData == 0){
	  towerId = w(0, 8);
	  if(towerId != 69){
	    std::cout << std::dec << "   ";
	    std::cout << " " << w(62, 2) << " " << w(48, 9) << " " << w(44, 1) << " " << w(32, 12) << " " << w(30, 2) << " " << w(28, 1) << " " << w(16, 12) << " " << w(8, 8) << " " << w(0, 8);
	    blockSize = w(48, 9);
	  }
	}
	else if(towerId != 69){
	  std::cout << std::dec << "   ";

	  if(iData % 3 == 1){
	    int adc3(w(48, 12));
	    int adc2(w(32, 12));
	    int adc1(w(16, 12));
	    std::cout << " " << w(62, 2) << " " << w(60, 2) << " " << adc3 << " " << w(44, 2) << " " << adc2 << " " << w(30, 2) << " " << w(28, 2) << " " << adc1 << " " << w(10, 1) << " " << w(9, 1) << " " << w(8, 1) << " " << w(4, 3) << " " << w(0, 3);
	    if(adc3 > 250 || adc2 > 250 || adc1 > 250) std::cout << " *";
	  }
	  else if(iData % 3 == 2){
	    int adc7(w(48, 12));
	    int adc6(w(32, 12));
	    int adc5(w(16, 12));
	    int adc4(w(0, 12));
	    std::cout << " " << w(62, 2) << " " << w(60, 2) << " " << adc7 << " " << w(44, 2) << " " << adc6 << " " << w(30, 2) << " " << w(28, 2) << " " << adc5 << " " << w(12, 2) << " " << adc4;
	    if(adc7 > 250 || adc6 > 250 || adc5 > 250 || adc4 > 250) std::cout << " *";
	  }
	  else{
	    int adc10(w(32, 12));
	    int adc9(w(16, 12));
	    int adc8(w(0, 12));
	    std::cout << " " << w(62, 2) << " " << w(46, 2) << " " << w(44, 2) << " " << adc10 << " " << w(30, 2) << " " << w(28, 2) << " " << adc9 << " " << w(12, 2) << " " << adc8;
	    if(adc10 > 250 || adc9 > 250 || adc8 > 250) std::cout << " *";
	  }
	}

	++iData;
	if(iData == blockSize) iData = 0;
      }

      std::cout << std::endl;
      std::cout << std::hex;
    }

    ++event;
  }

}
