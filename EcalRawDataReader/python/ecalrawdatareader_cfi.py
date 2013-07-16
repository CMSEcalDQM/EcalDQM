import FWCore.ParameterSet.Config as cms

ecalRawDataReader = cms.EDAnalyzer('EcalRawDataReader',
                                   FEDRawDataCollectionTag = cms.InputTag('RawDataCollector'),
                                   EcalRawDataCollectionTag = cms.InputTag('ecalEBunpacker'),
                                   fileName = cms.untracked.string("ecalRaw.root")                                   
)
