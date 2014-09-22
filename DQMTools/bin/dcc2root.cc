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
