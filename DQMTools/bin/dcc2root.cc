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

struct Arguments {
  TString collectionTag;
  TString outputName;

  Arguments() :
    collectionTag("rawDataCollector"),
    outputName("dcc.root")
  {}
};

bool makeNtuples(TFile&, Arguments const&);

void usage()
{
  std::cout << "Usage: dcc2root [-o outputName] input" << std::endl;
}

int
main(int argc, char** argv)
{
  Arguments args;

  opterr = 0;

  bool parseOpts(true);
  int opt(0);
  while(parseOpts){
    opt = getopt(argc, argv, ":o:h");
    switch(opt){
    case '?':
      std::cerr << "Unknown option -" << char(optopt) << std::endl;
      break;
    case ':':
      std::cerr << "Option -" << char(optopt) << " requires an argument" << std::endl;
      parseOpts = false;
      break;
    case 'o':
      args.outputName = optarg;
      break;
    case 'h':
      usage();
      return 0;
    default:
      parseOpts = false;
      break;
    }
  }

  if(optind != argc - 1){
    usage();
    return 1;
  }

  gSystem->Load("libFWCoreFWLite");
  AutoLibraryLoader::enable();

  TFile* input(TFile::Open(argv[optind]));
  if(!input){
    std::cerr << argv[optind] << " could not be opened." << std::endl;
    return 1;
  }

  int retval(makeNtuples(*input, args));

  delete input;

  return retval;
}
