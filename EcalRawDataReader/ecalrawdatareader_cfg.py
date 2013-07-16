import FWCore.ParameterSet.Config as cms

process = cms.Process("Demo")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.load("EventFilter.EcalRawToDigi.EcalUnpackerMapping_cfi")

process.load("EventFilter.EcalRawToDigi.EcalUnpackerData_cfi")

process.ecalEBunpacker.InputLabel = cms.InputTag("rawDataCollector")

process.load("DQM.Integration.test.FrontierCondition_GT_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )

process.source = cms.Source("NewEventStreamFileReader",
    fileNames = cms.untracked.vstring(
        'file:/data/lookarea/Data.00189099.0061.A.storageManager.03.0000.dat'
    )
)

process.reader = cms.EDAnalyzer('EcalRawDataReader',
                                FEDRawDataCollectionTag = cms.InputTag('rawDataCollector'),                                
                                EcalRawDataCollectionTag = cms.InputTag("ecalEBunpacker"),
                                fileName = cms.untracked.string("miniDAQRaw.root")
)


process.p = cms.Path(
    process.ecalEBunpacker *
    process.reader
    )
