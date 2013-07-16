import FWCore.ParameterSet.Config as cms

dqmFileLoader = cms.EDAnalyzer("DQMFileLoader",
                               directory = cms.untracked.string(''),
                               workflow = cms.untracked.string(''),
                               verbose = cms.untracked.bool(False)
                               )
