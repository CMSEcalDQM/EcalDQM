import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')
options._tags.pop('numEvent%d')
options._tagOrder.remove('numEvent%d')
options.parseArguments()

process = cms.Process('CONVERT')

process.options = cms.untracked.PSet(
    wantSummary = cms.untracked.bool(True)
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

process.source = cms.Source('NewEventStreamFileReader',
    fileNames = cms.untracked.vstring(options.inputFiles)
)

process.output = cms.OutputModule('PoolOutputModule',
    fileName = cms.untracked.string(options.outputFile)
)

process.load('FWCore.MessageService.MessageLogger_cfi')
process.MessageLogger.cerr.reportEvery = 100

process.outpath = cms.EndPath(process.output)
