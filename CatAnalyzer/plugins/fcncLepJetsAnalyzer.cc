// -*- C++ -*-
//
// Package:    fcncLepJetsAnalyzer
// Class:      fcncLepJetsAnalyzer
//
/**\class fcncLepJetsAnalyzer fcncLepJetsAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
// Original Author: Javier Brochero
// Author:  Jiwon Park
// Created:  2018

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "CATTools/DataFormats/interface/GenJet.h"
#include "CATTools/DataFormats/interface/GenTop.h"
#include "CATTools/DataFormats/interface/Muon.h"
#include "CATTools/DataFormats/interface/Electron.h"
#include "CATTools/DataFormats/interface/Jet.h"
#include "CATTools/DataFormats/interface/MET.h"
#include "CATTools/DataFormats/interface/GenWeights.h"

#include "CATTools/CommonTools/interface/ScaleFactorEvaluator.h"
#include "CATTools/CatAnalyzer/interface/BTagWeightEvaluator.h"
#include "CATTools/CommonTools/interface/AnalysisHelper.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "TH1.h"
#include "TTree.h"
#include "TLorentzVector.h"

using namespace cat;

class fcncLepJetsAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit fcncLepJetsAnalyzer(const edm::ParameterSet&);
  ~fcncLepJetsAnalyzer();

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);

  //----------------------------------------------------------------
  bool IsSelectMuon    (const cat::Muon     & i_muon_candidate);
  bool IsVetoMuon      (const cat::Muon     & i_muon_candidate);
  bool IsSelectElectron(const cat::Electron & i_electron_candidate);
  bool IsVetoElectron  (const cat::Electron & i_electron_candidate);

  bool isMC_ ;
  bool doLooseLepton_;

  int TTbarMC_; // 0->No ttbar, 1->ttbar Signal, 2->ttbar Background
  int TTbarCatMC_;
  unsigned int SkimNJets_;
  bool KFUsebtag_;
  bool CSVPosConKF_;
  // Trigger Names
  std::vector<string> triggerNameDataEl_;
  std::vector<string> triggerNameDataMu_;
  std::vector<string> triggerNameMCEl_;
  std::vector<string> triggerNameMCMu_;
  
  // Event Weights
  edm::EDGetTokenT<float>                        genWeightToken_;
  edm::EDGetTokenT<std::vector<float>>           pdfWeightToken_;
  edm::EDGetTokenT<std::vector<float>>           scaleUpWeightToken_;
  edm::EDGetTokenT<std::vector<float>>           scaleDownWeightToken_;
  edm::EDGetTokenT<float>                        puWeightToken_;
  edm::EDGetTokenT<float>                        puUpWeightToken_;
  edm::EDGetTokenT<float>                        puDownWeightToken_;
  edm::EDGetTokenT<float>                        deepCSVWeightToken_;
  edm::EDGetTokenT<std::vector<float>>           deepCSVSysWeightToken_;
  // Object Collections
  edm::EDGetTokenT<int>                          genttbarHiggsCatToken_;
  edm::EDGetTokenT<cat::GenTopCollection>        genttbarCatToken_;
  edm::EDGetTokenT<cat::MuonCollection>          muonToken_;
  edm::EDGetTokenT<cat::ElectronCollection>      electronToken_;
  edm::EDGetTokenT<cat::JetCollection>           jetToken_;
  edm::EDGetTokenT<cat::METCollection>           metToken_;
  edm::EDGetTokenT<int>                          pvToken_;
  // Trigger
  edm::EDGetTokenT<edm::TriggerResults> triggerBits_;
  edm::EDGetTokenT<edm::TriggerResults> triggerBits2_;
  edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjects_;
  // PU = 0 prescription
  edm::EDGetTokenT<int>                          nTrueVertToken_;

// ----------member data ---------------------------

  TTree *tree;

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  // Tree Branches
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------

  // Event info
  int b_Event, b_Run, b_Lumi_Number;
  float b_GenWeight;
  std::vector<float> *b_ScaleWeight, *b_PDFWeight;

  // PU/Vertices
  std::vector<float> *b_PUWeight;
  int b_nGoodPV, b_nTruePV;

  // Channel and Categorization
  int b_GenChannel, b_Channel, b_GenCone_NgJetsW, b_GenHiggsCatID;
  std::vector<float> *b_GenCone_gJet_pt, *b_GenCone_gJet_eta, *b_GenCone_gJet_phi, *b_GenCone_gJet_e;
  std::vector<int>   *b_GenConeCatID, *b_GenCone_gJetFlavW;

  // GEN Lepton and neutrino
  float b_GenLepton_pt, b_GenLepton_eta, b_GenLepton_phi, b_GenLepton_e;
  float b_GenNu_pt, b_GenNu_eta, b_GenNu_phi, b_GenNu_e;

  // Leptons and MET
  float b_Lepton_pt, b_Lepton_eta, b_Lepton_phi, b_Lepton_e, b_Lepton_relIso, b_Lepton_LES;
  float b_MET, b_MET_phi;
  bool  b_Lepton_isIso;
  std::vector<float> *b_Lepton_SF;

  //additional jets 
  float b_addbjet1_pt, b_addbjet1_eta, b_addbjet1_phi, b_addbjet1_e; 
  float b_addbjet2_pt, b_addbjet2_eta, b_addbjet2_phi, b_addbjet2_e; 
  float b_addHbjet1_pt, b_addHbjet1_eta, b_addHbjet1_phi, b_addHbjet1_e;
  float b_addHbjet2_pt, b_addHbjet2_eta, b_addHbjet2_phi, b_addHbjet2_e;
  float b_dRHbb;

  // Jets
  int b_Jet_Number;
  std::vector<float> *b_Jet_pt, *b_Jet_eta, *b_Jet_phi, *b_Jet_e;
  std::vector<int>   *b_Jet_Index;
  float               b_DRAddJets;

  // Jet Flavour
  std::vector<int> *b_Jet_partonFlavour;
  std::vector<int> *b_Jet_hadronFlavour;

  // JES and JER
  std::vector<float> *b_Jet_JES_Up, *b_Jet_JES_Down;
  std::vector<float> *b_Jet_JER_Up, *b_Jet_JER_Nom, *b_Jet_JER_Down;

  // b-Jet discriminant
  std::vector<float> *b_Jet_CSV, *b_Jet_deepCSV;
  std::vector<float> *b_Jet_SF_deepCSV_25;
  std::vector<float> *b_Jet_SF_deepCSV_30;
  std::vector<float> *b_Jet_SF_deepCSV_35;
  std::vector<float> *b_Jet_SF_deepCSV_40;
  std::vector<float> *b_Jet_SF_deepCSV_38;
  std::vector<float> *b_Jet_SF_deepCSV;

  // c-Jet discriminant
  std::vector<float> *b_Jet_CvsL, *b_Jet_CvsB;
  std::vector<float> *b_Jet_deepCvsL, *b_Jet_deepCvsB;

  // Histograms: Number of Events and Weights
  TH1D *EventInfo, *ScaleWeights;

  // Scale factor evaluators
  BTagWeightEvaluator SF_deepCSV_;
  ScaleFactorEvaluator SF_muon_, SF_elec_;
  
};

fcncLepJetsAnalyzer::fcncLepJetsAnalyzer(const edm::ParameterSet& iConfig):
  doLooseLepton_(iConfig.getUntrackedParameter<bool>("doLooseLepton", false)),
  TTbarMC_    (iConfig.getUntrackedParameter<int>("TTbarSampleLabel", 0)),
  TTbarCatMC_ (iConfig.getUntrackedParameter<int>("TTbarCatLabel", 0)),
  SkimNJets_  (iConfig.getUntrackedParameter<unsigned int>("Skim_N_Jets", 0)),
  KFUsebtag_  (iConfig.getUntrackedParameter<bool>("KFUsebtagLabel", true)),
  CSVPosConKF_(iConfig.getUntrackedParameter<bool>("CSVPosConKFLabel", true)),
  triggerNameDataEl_(iConfig.getUntrackedParameter<std::vector<string>>("triggerNameDataEl")),
  triggerNameDataMu_(iConfig.getUntrackedParameter<std::vector<string>>("triggerNameDataMu")),
  triggerNameMCEl_  (iConfig.getUntrackedParameter<std::vector<string>>("triggerNameMCEl")),
  triggerNameMCMu_  (iConfig.getUntrackedParameter<std::vector<string>>("triggerNameMCMu"))
{
  const auto elecSFSet = iConfig.getParameter<edm::ParameterSet>("elecSF");
  SF_elec_.set(elecSFSet.getParameter<std::vector<double>>("pt_bins" ),
	       elecSFSet.getParameter<std::vector<double>>("eta_bins"),
	       elecSFSet.getParameter<std::vector<double>>("values"),
	       elecSFSet.getParameter<std::vector<double>>("errors"));
  const auto muonSFSet = iConfig.getParameter<edm::ParameterSet>("muonSF");
  SF_muon_.set(muonSFSet.getParameter<std::vector<double>>("pt_bins"),
	       muonSFSet.getParameter<std::vector<double>>("abseta_bins"),
	       muonSFSet.getParameter<std::vector<double>>("values"),
	       muonSFSet.getParameter<std::vector<double>>("errors"));
  SF_deepCSV_.initCSVWeight(false, "deepcsv");

  // Weights
  auto genWeightLabel = iConfig.getParameter<edm::InputTag>("genWeightLabel");
  // aMC@NLO
  genWeightToken_        = consumes<float>             (edm::InputTag(genWeightLabel.label()));
  // PDF
  pdfWeightToken_       = consumes<std::vector<float>>(edm::InputTag(genWeightLabel.label(), "pdf"));
  // Scale
  scaleUpWeightToken_   = consumes<std::vector<float>>(edm::InputTag(genWeightLabel.label(), "scaleup"));
  scaleDownWeightToken_ = consumes<std::vector<float>>(edm::InputTag(genWeightLabel.label(), "scaledown"));
  // PileUp
  auto puWeightLabel = iConfig.getParameter<edm::InputTag>("puWeightLabel");
  puWeightToken_         = consumes<float>             (edm::InputTag(puWeightLabel.label()));
  puUpWeightToken_       = consumes<float>             (edm::InputTag(puWeightLabel.label(),"up"));
  puDownWeightToken_     = consumes<float>             (edm::InputTag(puWeightLabel.label(),"dn"));
  // CSV Weights
  auto deepcsvWeightLabel = iConfig.getParameter<edm::InputTag>("deepcsvWeightLabel");
  deepCSVWeightToken_        = consumes<float>             (edm::InputTag(deepcsvWeightLabel.label()));
  deepCSVSysWeightToken_     = consumes<std::vector<float>>(edm::InputTag(deepcsvWeightLabel.label(), "syst"));
  // GEN and ttbar Categorization
  genttbarCatToken_      = consumes<cat::GenTopCollection>        (iConfig.getParameter<edm::InputTag>("genttbarCatLabel"));
  genttbarHiggsCatToken_ = consumes<int>                          (iConfig.getParameter<edm::InputTag>("genHiggsCatLabel"));
  // Object Collections
  muonToken_         = consumes<cat::MuonCollection>          (iConfig.getParameter<edm::InputTag>("muonLabel"));
  electronToken_     = consumes<cat::ElectronCollection>      (iConfig.getParameter<edm::InputTag>("electronLabel"));
  jetToken_          = consumes<cat::JetCollection>           (iConfig.getParameter<edm::InputTag>("jetLabel"));
  metToken_          = consumes<cat::METCollection>           (iConfig.getParameter<edm::InputTag>("metLabel"));
  pvToken_           = consumes<int>                          (iConfig.getParameter<edm::InputTag>("pvLabel"));
  // Trigger  
  auto triggerLabel  = iConfig.getParameter<edm::InputTag>("triggerBits");
  triggerBits_       = consumes<edm::TriggerResults>                    (edm::InputTag(triggerLabel.label(),"","HLT"));
  triggerBits2_      = consumes<edm::TriggerResults>                    (edm::InputTag(triggerLabel.label(),"","HLT2"));
  triggerObjects_    = consumes<pat::TriggerObjectStandAloneCollection> (iConfig.getParameter<edm::InputTag>("triggerObjects"));
  // PU = 0 prescription
  nTrueVertToken_    = consumes<int>                           (iConfig.getParameter<edm::InputTag>("nTrueVertLabel"));
 
  b_PUWeight     = new std::vector<float>;
  b_PDFWeight    = new std::vector<float>;  
  b_ScaleWeight  = new std::vector<float>;  
  b_Lepton_SF    = new std::vector<float>;  

  b_GenConeCatID      = new std::vector<int>;
  b_GenCone_gJet_pt   = new std::vector<float>;
  b_GenCone_gJet_eta  = new std::vector<float>;
  b_GenCone_gJet_phi  = new std::vector<float>;
  b_GenCone_gJet_e    = new std::vector<float>;
  b_GenCone_gJetFlavW = new std::vector<int>;

  b_Jet_pt   = new std::vector<float>;
  b_Jet_eta   = new std::vector<float>;
  b_Jet_phi   = new std::vector<float>;
  b_Jet_e    = new std::vector<float>;
  b_Jet_Index= new std::vector<int>;
  b_Jet_CSV  = new std::vector<float>;
  b_Jet_deepCSV  = new std::vector<float>;
  b_Jet_SF_deepCSV_25  = new std::vector<float>;
  b_Jet_SF_deepCSV_30  = new std::vector<float>;
  b_Jet_SF_deepCSV_35  = new std::vector<float>;
  b_Jet_SF_deepCSV_40  = new std::vector<float>;
  b_Jet_SF_deepCSV_38  = new std::vector<float>;
  b_Jet_SF_deepCSV     = new std::vector<float>;
  b_Jet_CvsL    = new std::vector<float>;  
  b_Jet_CvsB    = new std::vector<float>;
  b_Jet_deepCvsL    = new std::vector<float>;
  b_Jet_deepCvsB    = new std::vector<float>; 
  b_Jet_partonFlavour = new std::vector<int>;
  b_Jet_hadronFlavour = new std::vector<int>;
  b_Jet_JES_Up   = new std::vector<float>;
  b_Jet_JES_Down = new std::vector<float>;
  b_Jet_JER_Up   = new std::vector<float>;
  b_Jet_JER_Nom  = new std::vector<float>;
  b_Jet_JER_Down = new std::vector<float>;

  usesResource("TFileService");
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree", "TopTree");

  tree->Branch("event",      &b_Event,       "event/I");
  tree->Branch("run",        &b_Run,         "run/I");
  tree->Branch("luminumber", &b_Lumi_Number, "luminumber/I");
  tree->Branch("genweight",  &b_GenWeight,   "genweight/F");
  tree->Branch("TruePV",     &b_nTruePV,     "TruePV/I");
  tree->Branch("GoodPV",     &b_nGoodPV,     "GoodPV/I");
  tree->Branch("channel",    &b_Channel,     "channel/I");
  tree->Branch("PUWeight",   "std::vector<float>", &b_PUWeight);

  tree->Branch("MET",     &b_MET,     "MET/F");
  tree->Branch("MET_phi", &b_MET_phi, "MET_phi/F");
  tree->Branch("lepton_pt",  &b_Lepton_pt,  "lepton_pt/F");
  tree->Branch("lepton_eta", &b_Lepton_eta, "lepton_eta/F");
  tree->Branch("lepton_phi", &b_Lepton_phi, "lepton_phi/F");
  tree->Branch("lepton_e" ,  &b_Lepton_e,   "lepton_e/F" );
  tree->Branch("lepton_LES", &b_Lepton_LES, "lepton_LES/F" );
  tree->Branch("lepton_SF",  "std::vector<float>", &b_Lepton_SF );
  tree->Branch("lepton_relIso", &b_Lepton_relIso, "lepton_relIso/F");
  tree->Branch("lepton_isIso",  &b_Lepton_isIso,  "lepton_isIso/O");

  tree->Branch("jet_pt",           "std::vector<float>", &b_Jet_pt);
  tree->Branch("jet_eta",          "std::vector<float>", &b_Jet_eta);
  tree->Branch("jet_phi",          "std::vector<float>", &b_Jet_phi);
  tree->Branch("jet_e" ,           "std::vector<float>", &b_Jet_e );
  tree->Branch("jet_index" ,       "std::vector<int>",   &b_Jet_Index );
  tree->Branch("jet_CSV" ,         "std::vector<float>", &b_Jet_CSV );
  tree->Branch("jet_deepCSV",      "std::vector<float>", &b_Jet_deepCSV );
  tree->Branch("jet_SF_deepCSV_25","std::vector<float>", &b_Jet_SF_deepCSV_25 );
  tree->Branch("jet_SF_deepCSV_30","std::vector<float>", &b_Jet_SF_deepCSV_30 );
  tree->Branch("jet_SF_deepCSV_35","std::vector<float>", &b_Jet_SF_deepCSV_35 );
  tree->Branch("jet_SF_deepCSV_40","std::vector<float>", &b_Jet_SF_deepCSV_40 );
  tree->Branch("jet_SF_deepCSV_38","std::vector<float>", &b_Jet_SF_deepCSV_38 );
  tree->Branch("jet_SF_deepCSV",   "std::vector<float>", &b_Jet_SF_deepCSV );
  tree->Branch("jet_CvsL",         "std::vector<float>", &b_Jet_CvsL );
  tree->Branch("jet_CvsB",         "std::vector<float>", &b_Jet_CvsB );
  tree->Branch("jet_deepCvsL",     "std::vector<float>", &b_Jet_deepCvsL );
  tree->Branch("jet_deepCvsB",     "std::vector<float>", &b_Jet_deepCvsB );

  tree->Branch("jet_number" , &b_Jet_Number, "jet_number/I" );
  tree->Branch("jet_partonFlavour", "std::vector<int>",   &b_Jet_partonFlavour);
  tree->Branch("jet_hadronFlavour", "std::vector<int>",   &b_Jet_hadronFlavour);
  tree->Branch("jet_JES_Up",        "std::vector<float>", &b_Jet_JES_Up );
  tree->Branch("jet_JES_Down",      "std::vector<float>", &b_Jet_JES_Down );
  tree->Branch("jet_JER_Up",        "std::vector<float>", &b_Jet_JER_Up );
  tree->Branch("jet_JER_Nom",       "std::vector<float>", &b_Jet_JER_Nom );
  tree->Branch("jet_JER_Down",      "std::vector<float>", &b_Jet_JER_Down );

  //add bjets from Higgs
  tree->Branch("addHbjet1_pt",  &b_addHbjet1_pt,  "addHbjet1_pt/F");
  tree->Branch("addHbjet1_eta", &b_addHbjet1_eta, "addHbjet1_eta/F");
  tree->Branch("addHbjet1_phi", &b_addHbjet1_phi, "addHbjet1_phi/F");
  tree->Branch("addHbjet1_e",   &b_addHbjet1_e,   "addHbjet1_e/F");
  tree->Branch("addHbjet2_pt",  &b_addHbjet2_pt,  "addHbjet2_pt/F");
  tree->Branch("addHbjet2_eta", &b_addHbjet2_eta, "addHbjet2_eta/F");
  tree->Branch("addHbjet2_phi", &b_addHbjet2_phi, "addHbjet2_phi/F");
  tree->Branch("addHbjet2_e",   &b_addHbjet2_e,   "addHbjet2_e/F");
  tree->Branch("dRHbb",         &b_dRHbb,         "dRHbb/F");


  // GEN Variables (only ttbarSignal)
  if(TTbarMC_ == 1){
    tree->Branch("pdfweight",   "std::vector<float>", &b_PDFWeight );
    tree->Branch("scaleweight", "std::vector<float>", &b_ScaleWeight );

    tree->Branch("genconecatid" ,      "std::vector<int>",   &b_GenConeCatID);
    tree->Branch("gencone_gjet_pt" ,   "std::vector<float>", &b_GenCone_gJet_pt);
    tree->Branch("gencone_gjet_eta" ,  "std::vector<float>", &b_GenCone_gJet_eta);
    tree->Branch("gencone_gjet_phi" ,  "std::vector<float>", &b_GenCone_gJet_phi);
    tree->Branch("gencone_gjet_e" ,    "std::vector<float>", &b_GenCone_gJet_e);
    tree->Branch("gencone_gJetFlavW" , "std::vector<int>",   &b_GenCone_gJetFlavW);
    tree->Branch("gencone_NgjetsW", &b_GenCone_NgJetsW, "gencone_NgjetsW/I");
    tree->Branch("draddjets",       &b_DRAddJets,       "draddjets/F");
    tree->Branch("genhiggscatid",   &b_GenHiggsCatID,   "genhiggscatid/I");

    tree->Branch("genchannel",    &b_GenChannel,    "genchannel/I");
    tree->Branch("genlepton_pt",  &b_GenLepton_pt,  "genlepton_pt/F");
    tree->Branch("genlepton_eta", &b_GenLepton_eta, "genlepton_eta/F");
    tree->Branch("genlepton_phi", &b_GenLepton_phi, "genlepton_phi/F");
    tree->Branch("genlepton_e",   &b_GenLepton_e,   "genlepton_e/F");
    tree->Branch("gennu_pt",      &b_GenNu_pt,      "gennu_pt/F");
    tree->Branch("gennu_eta",     &b_GenNu_eta,     "gennu_eta/F");
    tree->Branch("gennu_phi",     &b_GenNu_phi,     "gennu_phi/F");
    tree->Branch("gennu_e",       &b_GenNu_e,       "gennu_e/F");

    tree->Branch("addbjet1_pt",  &b_addbjet1_pt, "addbjet1_pt/F"); 
    tree->Branch("addbjet1_eta", &b_addbjet1_eta, "addbjet1_eta/F"); 
    tree->Branch("addbjet1_phi", &b_addbjet1_phi, "addbjet1_phi/F"); 
    tree->Branch("addbjet1_e",   &b_addbjet1_e, "addbjet1_e/F"); 
    tree->Branch("addbjet2_pt",  &b_addbjet2_pt, "addbjet2_pt/F");
    tree->Branch("addbjet2_eta", &b_addbjet2_eta, "addbjet2_eta/F");
    tree->Branch("addbjet2_phi", &b_addbjet2_phi, "addbjet2_phi/F");
    tree->Branch("addbjet2_e",   &b_addbjet2_e, "addbjet2_e/F");  
  }

  EventInfo = fs->make<TH1D>("EventInfo","Event Information",9,0,9);
  EventInfo->GetXaxis()->SetBinLabel(1,"Number of Events");
  EventInfo->GetXaxis()->SetBinLabel(2,"Sum of Weights");
  EventInfo->GetXaxis()->SetBinLabel(3,"ttbb Events pT>20");
  EventInfo->GetXaxis()->SetBinLabel(4,"ttbb Lep Events pT>20");
  EventInfo->GetXaxis()->SetBinLabel(5,"ttbb Lep (tau lep decay) Events pT>20");
  EventInfo->GetXaxis()->SetBinLabel(6,"ttjj Events pT>20");
  EventInfo->GetXaxis()->SetBinLabel(7,"ttjj Lep Events pT>20");
  EventInfo->GetXaxis()->SetBinLabel(8,"ttjj Lep (tau lep decay) Events pT>20");
  EventInfo->GetXaxis()->SetBinLabel(9,"Sum of PU Weights");

  ScaleWeights = fs->make<TH1D>("ScaleWeights","Event Weights:",6,0,6);
  ScaleWeights->GetXaxis()->SetBinLabel(1,"muR=Nom  muF=Up");
  ScaleWeights->GetXaxis()->SetBinLabel(2,"muR=Nom  muF=Down");
  ScaleWeights->GetXaxis()->SetBinLabel(3,"muR=Up   muF=Nom");
  ScaleWeights->GetXaxis()->SetBinLabel(4,"muR=Up   muF=Up");
  ScaleWeights->GetXaxis()->SetBinLabel(5,"muR=Down muF=Nom");
  ScaleWeights->GetXaxis()->SetBinLabel(6,"muR=Down muF=Down");

}


fcncLepJetsAnalyzer::~fcncLepJetsAnalyzer()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
  delete b_PUWeight;
  delete b_PDFWeight;
  delete b_ScaleWeight;

  delete b_GenConeCatID;
  delete b_GenCone_gJet_pt;
  delete b_GenCone_gJet_eta;
  delete b_GenCone_gJet_phi;
  delete b_GenCone_gJet_e;
  delete b_GenCone_gJetFlavW;

  delete b_Lepton_SF;
  delete b_Jet_pt;
  delete b_Jet_eta;
  delete b_Jet_phi;
  delete b_Jet_e;
  delete b_Jet_Index;

  delete b_Jet_partonFlavour;
  delete b_Jet_hadronFlavour;
  delete b_Jet_JES_Up;
  delete b_Jet_JES_Down;
  delete b_Jet_JER_Up;
  delete b_Jet_JER_Nom;
  delete b_Jet_JER_Down;

  delete b_Jet_SF_deepCSV_25;
  delete b_Jet_SF_deepCSV_30;
  delete b_Jet_SF_deepCSV_35;
  delete b_Jet_SF_deepCSV_40;
  delete b_Jet_SF_deepCSV_38;
  delete b_Jet_SF_deepCSV;

  delete b_Jet_CSV;
  delete b_Jet_CvsL;
  delete b_Jet_CvsB;
  delete b_Jet_deepCSV;
  delete b_Jet_deepCvsL;
  delete b_Jet_deepCvsB;

}

// ------------ method called for each event  ------------
void fcncLepJetsAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  b_PUWeight   ->clear();
  b_ScaleWeight->clear();
  b_PDFWeight  ->clear();

  b_GenConeCatID->clear();
  b_GenCone_gJet_pt->clear();
  b_GenCone_gJet_eta->clear();
  b_GenCone_gJet_phi->clear();
  b_GenCone_gJet_e->clear();
  b_GenCone_gJetFlavW->clear();

  b_Lepton_SF->clear();

  b_Jet_pt    ->clear();
  b_Jet_eta   ->clear();
  b_Jet_phi   ->clear();
  b_Jet_e     ->clear();
  b_Jet_Index->clear();

  b_Jet_partonFlavour->clear();
  b_Jet_hadronFlavour->clear();
  b_Jet_JES_Up  ->clear();
  b_Jet_JES_Down->clear();
  b_Jet_JER_Up  ->clear();
  b_Jet_JER_Nom ->clear();
  b_Jet_JER_Down->clear();

  b_Jet_CSV          ->clear();
  b_Jet_deepCSV      ->clear();
  b_Jet_SF_deepCSV_25->clear();
  b_Jet_SF_deepCSV_30->clear();
  b_Jet_SF_deepCSV_35->clear();
  b_Jet_SF_deepCSV_40->clear();
  b_Jet_SF_deepCSV_38->clear();
  b_Jet_SF_deepCSV   ->clear();
  b_Jet_CvsL         ->clear();
  b_Jet_CvsB         ->clear();
  b_Jet_deepCvsL     ->clear();
  b_Jet_deepCvsB     ->clear();

  b_addbjet1_pt  = b_addbjet1_e   = b_addbjet2_pt  = b_addbjet2_e   = -1.0 ;
  b_addbjet1_eta = b_addbjet1_phi = b_addbjet2_eta = b_addbjet2_phi = -10.0;
  b_addHbjet1_pt  = b_addHbjet1_e   = b_addHbjet2_pt  = b_addHbjet2_e   = -1.0;
  b_addHbjet1_eta = b_addHbjet1_phi = b_addHbjet2_eta = b_addHbjet2_phi = -10.0;
  b_dRHbb = -1.0;

  //---------------------------------------------------------------------------
  // Event Info
  //---------------------------------------------------------------------------
  isMC_ = !iEvent.isRealData();

  b_Event        = iEvent.id().event();
  b_Run          = iEvent.id().run();
  b_Lumi_Number  = iEvent.luminosityBlock();
  b_Lepton_relIso = 999;
  b_Lepton_isIso = false;

  EventInfo->Fill(0.5, 1.0);         // Number of Events

  if(isMC_) {
    //---------------------------------------------------------------------------
    // PU Info
    //---------------------------------------------------------------------------
    edm::Handle<int> nTrueVertHandle;
    iEvent.getByToken( nTrueVertToken_, nTrueVertHandle );
    b_nTruePV = *nTrueVertHandle;

    edm::Handle<float> PUWeight;
    iEvent.getByToken(puWeightToken_, PUWeight);
    b_PUWeight->push_back(*PUWeight); // Central

    EventInfo->Fill(8.5, *PUWeight); // Sum of PUWeights

    edm::Handle<float> PUWeight_Up;
    iEvent.getByToken(puUpWeightToken_, PUWeight_Up);
    b_PUWeight->push_back(*PUWeight_Up); //Syst. Up

    edm::Handle<float> PUWeight_Down;
    iEvent.getByToken(puDownWeightToken_, PUWeight_Down);
    b_PUWeight->push_back(*PUWeight_Down); //Syst. Up

    //---------------------------------------------------------------------------
    // Weights at Generation Level: aMC@NLO
    //---------------------------------------------------------------------------
    edm::Handle<float> genWeight;
    iEvent.getByToken(genWeightToken_, genWeight);
    b_GenWeight = *genWeight;

    EventInfo->Fill(1.5, b_GenWeight); // Sum of aMC@NLO Weights
  }
  else{
    b_PUWeight->push_back(1.0);
    b_GenWeight = 1.0;
    b_nTruePV = 0;
  }

  //---------------------------------------------------------------------------
  // Weights for Syst. Scale and PDF: ttbar
  //---------------------------------------------------------------------------

  if(TTbarMC_ == 1 ) {

    edm::Handle<std::vector<float>> scaleUpWeightsHandle, scaleDownWeightsHandle;
    iEvent.getByToken(scaleUpWeightToken_,   scaleUpWeightsHandle);
    iEvent.getByToken(scaleDownWeightToken_, scaleDownWeightsHandle);


    // muR/muF Scale Weights
    b_ScaleWeight->push_back(scaleUpWeightsHandle  ->at(0)); // muR=Nom  muF=Up
    b_ScaleWeight->push_back(scaleDownWeightsHandle->at(0)); // muR=Nom  muF=Down
    b_ScaleWeight->push_back(scaleUpWeightsHandle  ->at(1)); // muR=Up   muF=Nom
    b_ScaleWeight->push_back(scaleUpWeightsHandle  ->at(2)); // muR=Up   muF=Up
    b_ScaleWeight->push_back(scaleDownWeightsHandle->at(1)); // muR=Down muF=Nom
    b_ScaleWeight->push_back(scaleDownWeightsHandle->at(2)); // muR=Down muF=Down

    // Sum of muR/muF Scale Weights
    for(unsigned int iscale = 0; iscale< b_ScaleWeight->size(); iscale++)
      ScaleWeights->Fill(iscale, b_ScaleWeight->at(iscale)); 

    edm::Handle<std::vector<float>> PDFWeightsHandle;
    iEvent.getByToken(pdfWeightToken_,   PDFWeightsHandle);

    for ( auto& w : *PDFWeightsHandle ) b_PDFWeight->push_back(w);

  }

  //---------------------------------------------------------------------------
  // Generated Particles (For Pythia8)
  //---------------------------------------------------------------------------
  int nGenLep  = 999;
  bool IsCat = false;
  if(TTbarMC_ > 0) {
    //---------------------------------------------------------------------------
    // Event Categorization Using Higgs Code
    // // Twiki: https://twiki.cern.ch/twiki/bin/view/CMSPublic/GenHFHadronMatcher
    //---------------------------------------------------------------------------
    edm::Handle<int> genttbarHiggsCatHandle;
    iEvent.getByToken( genttbarHiggsCatToken_, genttbarHiggsCatHandle );
    b_GenHiggsCatID = *genttbarHiggsCatHandle;

    //---------------------------------------------------------------------------
    // Event Categorization Using Cone
    //---------------------------------------------------------------------------
    edm::Handle<cat::GenTopCollection> genttbarConeCat;
    iEvent.getByToken( genttbarCatToken_, genttbarConeCat );

    // [0]: Decay mode. Just as a Cross Check!
    b_GenConeCatID->push_back(genttbarConeCat->begin()->semiLeptonic(0));
    // [1]: Number of Jets
    b_GenConeCatID->push_back(genttbarConeCat->begin()->NJets20());
    // [2]: Number of b-Jets
    b_GenConeCatID->push_back(genttbarConeCat->begin()->NbJets20());
    // [3]: Number of c-Jets
    b_GenConeCatID->push_back(genttbarConeCat->begin()->NcJets20());
    // [4]: Number of b-Jets Not comming from the top
    b_GenConeCatID->push_back(genttbarConeCat->begin()->NbJets20NoTop());
    // [5]: Number of add Jets
    b_GenConeCatID->push_back(genttbarConeCat->begin()-> NaddJets20());
    // [6]: Number of add b-Jets
    b_GenConeCatID->push_back(genttbarConeCat->begin()-> NaddbJets20());
    // [7]: Number of add c-Jets
    b_GenConeCatID->push_back(genttbarConeCat->begin()-> NaddcJets20());

    //-----------------
    //add b jets from H
    //-----------------
    b_addHbjet1_pt  = genttbarConeCat->begin()->HbJets1().Pt();
    b_addHbjet1_eta = genttbarConeCat->begin()->HbJets1().Eta();
    b_addHbjet1_phi = genttbarConeCat->begin()->HbJets1().Phi();
    b_addHbjet1_e   = genttbarConeCat->begin()->HbJets1().E();
    b_addHbjet2_pt  = genttbarConeCat->begin()->HbJets2().Pt();
    b_addHbjet2_eta = genttbarConeCat->begin()->HbJets2().Eta();
    b_addHbjet2_phi = genttbarConeCat->begin()->HbJets2().Phi();
    b_addHbjet2_e   = genttbarConeCat->begin()->HbJets2().E();
    b_dRHbb         = genttbarConeCat->begin()->dRbJetsFromHiggs();

    // additional Jets Information (p4)
    math::XYZTLorentzVector gJetGenCone[6];
    gJetGenCone[0] = genttbarConeCat->begin()->bJetsFromTop1();
    gJetGenCone[1] = genttbarConeCat->begin()->bJetsFromTop2();
    gJetGenCone[2] = genttbarConeCat->begin()->JetsFromW1();
    gJetGenCone[3] = genttbarConeCat->begin()->JetsFromW2();
    gJetGenCone[4] = genttbarConeCat->begin()->addJets1();
    gJetGenCone[5] = genttbarConeCat->begin()->addJets2();

    b_GenCone_NgJetsW = genttbarConeCat->begin()-> NWJets();
    b_GenCone_gJetFlavW->push_back(genttbarConeCat->begin()-> Wquarkflav1());
    b_GenCone_gJetFlavW->push_back(genttbarConeCat->begin()-> Wquarkflav2());

    for (int ijGT=0; ijGT<6; ijGT++){
      b_GenCone_gJet_pt  ->push_back(gJetGenCone[ijGT].Pt());
      b_GenCone_gJet_eta ->push_back(gJetGenCone[ijGT].Eta());
      b_GenCone_gJet_phi ->push_back(gJetGenCone[ijGT].Phi());
      b_GenCone_gJet_e   ->push_back(gJetGenCone[ijGT].E());
    }

    // adding additional b jet four-momentum
    b_addbjet1_pt  = genttbarConeCat->begin()->addbJets1().Pt();
    b_addbjet1_eta = genttbarConeCat->begin()->addbJets1().Eta();
    b_addbjet1_phi = genttbarConeCat->begin()->addbJets1().Phi();
    b_addbjet1_e   = genttbarConeCat->begin()->addbJets1().E();
    b_addbjet2_pt  = genttbarConeCat->begin()->addbJets2().Pt();
    b_addbjet2_eta = genttbarConeCat->begin()->addbJets2().Eta();
    b_addbjet2_phi = genttbarConeCat->begin()->addbJets2().Phi();
    b_addbjet2_e   = genttbarConeCat->begin()->addbJets2().E();
    b_DRAddJets    = genttbarConeCat->begin()->dRaddJets();

    if(genttbarConeCat->begin()-> NaddbJets20() > 1){
      EventInfo->Fill(2.5, 1.0); // Number of ttbb Events	
      if(genttbarConeCat->begin()->semiLeptonic(-1)) EventInfo->Fill(3.5, 1.0); // Number of ttbb Events (Includes tau) 
      if(genttbarConeCat->begin()->semiLeptonic(0))  EventInfo->Fill(4.5, 1.0); // Number of ttbb Events (Includes tau leptonic decay) 
    }
    if(genttbarConeCat->begin()-> NaddJets20() > 1){
      EventInfo->Fill(5.5, 1.0); // Number of ttjj Events	
      if(genttbarConeCat->begin()->semiLeptonic(-1)) EventInfo->Fill(6.5, 1.0); // Number of ttjj Events (Includes tau) 
      if(genttbarConeCat->begin()->semiLeptonic(0))  EventInfo->Fill(7.5, 1.0); // Number of ttjj Events (Includes tau leptonic decay) 
    }
    
    
    //---------------------------------------------------------------------------
    // Using the GenChannel from GenTop categorization
    //---------------------------------------------------------------------------
    nGenLep = genttbarConeCat->begin()->semiLeptonic(0); // semiLeptonic(0) includes tau leptonic decay
    //---------------------------------------------------------------------------

    // Category
    bool Isttjj = false;
    bool Isttbb = false;
    bool Isttb  = false;
    bool Isttcc = false;
    bool IsttLF = false;
    bool Istt   = false;

    // Categorization based in the Full Ph-Sp
    // Requires ttjj events to be categorized
    if(genttbarConeCat->begin()->NaddJets20() > 1) Isttjj = true;

    if      (Isttjj && genttbarConeCat->begin()->NaddbJets20() > 1) Isttbb = true;
    else if (Isttjj && genttbarConeCat->begin()->NaddbJets20() > 0) Isttb  = true;
    else if (Isttjj && genttbarConeCat->begin()->NaddcJets20() > 1) Isttcc = true;
    else if (Isttjj && genttbarConeCat->begin()->NaddJets20()  > 1) IsttLF = true;
    else Istt = true;

    // Categorization based in the Visible Ph-Sp
    // if(genttbarConeCat->begin()->NbJets20() > 1 && 
    //    genttbarConeCat->begin()->NJets20()  > 5) Isttjj = true;

    // if      (genttbarConeCat->begin()->NbJets20() > 3  && 
    // 	     genttbarConeCat->begin()->NJets20()  > 5) Isttbb = true;
    // else if (genttbarConeCat->begin()->NbJets20() > 2  && 
    // 	     genttbarConeCat->begin()->NJets20()  > 5) Isttb  = true;
    // else if (genttbarConeCat->begin()->NbJets20() > 1  && 
    // 	     genttbarConeCat->begin()->NJets20()  > 5  && 
    // 	     genttbarConeCat->begin()->NcJets20() > 1) Isttcc = true;
    // else if (genttbarConeCat->begin()->NbJets20() > 1  && 
    // 	     genttbarConeCat->begin()->NJets20()  > 5) IsttLF = true;
    // else Istt = true;

    if (TTbarCatMC_ == 0 || TTbarCatMC_ == 7) IsCat = true;
    if (Isttbb && TTbarCatMC_ == 1) IsCat = true;
    if (Isttb  && TTbarCatMC_ == 2) IsCat = true;
    if (Isttcc && TTbarCatMC_ == 3) IsCat = true;
    if (IsttLF && TTbarCatMC_ == 4) IsCat = true;
    if (Istt   && TTbarCatMC_ == 5) IsCat = true;
    if (Isttjj && TTbarCatMC_ == 6) IsCat = true;

    if(isMC_ && TTbarMC_== 1 && IsCat){
      if(nGenLep == 1){
        if (genttbarConeCat->begin()->semiLeptonicMuo())      b_GenChannel = 0;
        else if (genttbarConeCat->begin()->semiLeptonicEle()) b_GenChannel = 1;
        
        if(genttbarConeCat->begin()->lepton1().pt() != 0.){
          b_GenLepton_pt  = genttbarConeCat->begin()->lepton1().Pt();
          b_GenLepton_eta = genttbarConeCat->begin()->lepton1().Eta();
          b_GenLepton_phi = genttbarConeCat->begin()->lepton1().Phi();
          b_GenLepton_e   = genttbarConeCat->begin()->lepton1().E();
          b_GenNu_pt      = genttbarConeCat->begin()->nu1().Pt();
          b_GenNu_eta     = genttbarConeCat->begin()->nu1().Eta();
          b_GenNu_phi     = genttbarConeCat->begin()->nu1().Phi();
          b_GenNu_e       = genttbarConeCat->begin()->nu1().E();
        }
        else{
          b_GenLepton_pt  = genttbarConeCat->begin()->lepton2().Pt();
          b_GenLepton_eta = genttbarConeCat->begin()->lepton2().Eta();
          b_GenLepton_phi = genttbarConeCat->begin()->lepton2().Phi();
          b_GenLepton_e   = genttbarConeCat->begin()->lepton2().E();
          b_GenNu_pt      = genttbarConeCat->begin()->nu2().Pt();
          b_GenNu_eta     = genttbarConeCat->begin()->nu2().Eta();
          b_GenNu_phi     = genttbarConeCat->begin()->nu2().Phi();
          b_GenNu_e       = genttbarConeCat->begin()->nu2().E();
        }
      } // if(nGenLep == 1)
    }// if(GENTTbarMCTree_)
  } // if(TTbarMC>0)

  //---------------------------------------------------------------------------
  // Primary Vertex Info
  //---------------------------------------------------------------------------
  edm::Handle<int> pvHandle;
  iEvent.getByToken( pvToken_, pvHandle );
  b_nGoodPV = *pvHandle;

  //---------------------------------------------------------------------------
  // Missing E_T
  //---------------------------------------------------------------------------
  Handle<cat::METCollection> MET;
  iEvent.getByToken(metToken_, MET);

  // MET-PF
  b_MET     = MET->begin()->pt();
  b_MET_phi = MET->begin()->phi();

  //---------------------------------------------------------------------------
  // Electrons
  //---------------------------------------------------------------------------
  cat::ElectronCollection selectedElectrons;
  cat::ElectronCollection vetoElectrons;

  Handle<cat::ElectronCollection> electrons;
  iEvent.getByToken(electronToken_, electrons);

  //int ele32 = 0;

  if ( !doLooseLepton_ ) {
    for (unsigned int i = 0; i < electrons->size() ; i++) {
      const cat::Electron & electron = electrons->at(i);

      //if ( electron.isHLT_Ele32_WPTight() ) ele32 += 1;

      if ( IsSelectElectron( electron ) ) selectedElectrons.push_back( electron );
      else if ( IsVetoElectron( electron ) ) vetoElectrons.push_back( electron ); // does not Include selected electrons
    }
  }
  else if ( !electrons->empty() ) {
    if ( IsSelectElectron(electrons->at(0)) ) selectedElectrons.push_back( electrons->at(0) );
    for (unsigned int i = 1; i < electrons->size() ; i++) {
      const cat::Electron & electron = electrons->at(i);
      if ( IsVetoElectron(electron) ) vetoElectrons.push_back(electron);
    }
  }

  //---------------------------------------------------------------------------
  // Muons
  //---------------------------------------------------------------------------
  cat::MuonCollection selectedMuons;
  cat::MuonCollection vetoMuons;

  Handle<cat::MuonCollection> muons;
  iEvent.getByToken(muonToken_, muons);

  if ( !doLooseLepton_ ) {
    for (unsigned int i = 0; i < muons->size() ; i++) {
      const cat::Muon & muon = muons->at(i);

      if( IsSelectMuon( muon) ) selectedMuons.push_back( muon);
      else if( IsVetoMuon( muon) ) vetoMuons.push_back( muon); // does not Include selected muons
    }
  }
  else if ( !muons->empty() ) {
    if ( IsSelectMuon(muons->at(0)) ) selectedMuons.push_back( muons->at(0) );
    for (unsigned int i = 1; i < muons->size() ; i++) {
      const cat::Muon & muon = muons->at(i);
      if ( IsVetoMuon(muon) ) vetoMuons.push_back(muon);
    }
  }

  //---------------------------------------------------------------------------
  // Channel Selection
  //---------------------------------------------------------------------------
  TLorentzVector lepton;
  std::vector<float> *lepton_SF;
  lepton_SF = new std::vector<float>;

  float lepton_LES = 0.0;
  int ch_tag  = 999;

  if( selectedMuons.size()     == 1 &&
      vetoMuons.size()         == 0 &&
      selectedElectrons.size() == 0 &&
      vetoElectrons.size()     == 0){
    lepton.SetPtEtaPhiE(selectedMuons[0].pt(), selectedMuons[0].eta(), selectedMuons[0].phi(), selectedMuons[0].energy());
    ch_tag = 0; //muon + jets
    b_Lepton_relIso = selectedMuons[0].relIso(0.4);
    b_Lepton_isIso = (b_Lepton_relIso < 0.15);

    if(isMC_) {
      // Lepton SF (ID/ISO)
      lepton_SF->push_back( SF_muon_( selectedMuons[0].pt(), selectedMuons[0].eta() ) );       // [0]-> SF
      lepton_SF->push_back( SF_muon_( selectedMuons[0].pt(), selectedMuons[0].eta(),  1.0 ) ); // [1]-> SF+Error
      lepton_SF->push_back( SF_muon_( selectedMuons[0].pt(), selectedMuons[0].eta(), -1.0 ) ); // [2]-> SF-Error
      //LES
      lepton_LES = selectedMuons[0].shiftedEn();
    }
  }

  if( selectedMuons.size()     == 0 &&
      vetoMuons.size()         == 0 &&
      selectedElectrons.size() == 1 &&
      vetoElectrons.size()     == 0){
    ch_tag = 1; //electron + jets
    lepton.SetPtEtaPhiE(selectedElectrons[0].pt(), selectedElectrons[0].eta(), selectedElectrons[0].phi(), selectedElectrons[0].energy());
    b_Lepton_relIso = selectedElectrons.at(0).relIso(0.3);
    b_Lepton_isIso = (b_Lepton_relIso < (std::abs(selectedElectrons[0].scEta()) <= 1.479 ? 0.0766 : 0.0678) );

    if(isMC_) {
      // Lepton SF (ID/ISO)
      lepton_SF->push_back( SF_elec_( selectedElectrons[0].pt(), selectedElectrons[0].scEta() ) );       // [0]-> SF
      lepton_SF->push_back( SF_elec_( selectedElectrons[0].pt(), selectedElectrons[0].scEta(),  1.0 ) ); // [1]-> SF+Error
      lepton_SF->push_back( SF_elec_( selectedElectrons[0].pt(), selectedElectrons[0].scEta(), -1.0 ) ); // [2]-> SF-Error
      //LES
      lepton_LES = selectedElectrons[0].shiftedEn();
    }
  }

  //---------------------------------------------------------------------------
  // HLTrigger
  //---------------------------------------------------------------------------
  std::vector<string> triggerNameEl_, triggerNameMu_;
  if(isMC_){
    triggerNameEl_ = triggerNameMCEl_;
    triggerNameMu_ = triggerNameMCMu_;
  } 
  else{
    triggerNameEl_ = triggerNameDataEl_;
    triggerNameMu_ = triggerNameDataMu_;  
  }  
  
  bool EvTrigger = false; 
  edm::Handle<edm::TriggerResults> triggerBits;
  edm::Handle<pat::TriggerObjectStandAloneCollection> triggerObjects;
  iEvent.getByToken(triggerBits_, triggerBits);
  if( !iEvent.getByToken(triggerBits_, triggerBits) ){
    iEvent.getByToken(triggerBits2_, triggerBits);
  } 
  iEvent.getByToken(triggerObjects_, triggerObjects);
  const edm::TriggerNames &triggerNames = iEvent.triggerNames(*triggerBits);
  AnalysisHelper trigHelper = AnalysisHelper(triggerNames, triggerBits, triggerObjects);

  bool IsTriggerMu = false;
  bool IsTriggerEl = false;
  
  // DEBUG: Print all triggers
  // trigHelper.listFiredTriggers();

  for(std::vector<string>::iterator TrMu_it = triggerNameMu_.begin(); TrMu_it != triggerNameMu_.end(); TrMu_it++){
    IsTriggerMu = trigHelper.triggerFired(*TrMu_it);
    // No trigger
    if(*TrMu_it == "notrigger") IsTriggerMu = true; 
    if (IsTriggerMu) break;
  }
  
  for(std::vector<string>::iterator TrEl_it = triggerNameEl_.begin(); TrEl_it != triggerNameEl_.end(); TrEl_it++){
    IsTriggerEl = trigHelper.triggerFired(*TrEl_it);
    // No trigger
    if(*TrEl_it == "notrigger") IsTriggerEl = true;
    if (IsTriggerEl) break;
  }

  //if(ele32 > 0) IsTriggerEl = true;
  //else IsTriggerEl = false;

  if ((ch_tag == 0 && IsTriggerMu) ||
      (ch_tag == 1 && IsTriggerEl)) {
    EvTrigger = true;
  }

  //---------------------------------------------------------------------------
  // Fill Tree with events that have ONLY one lepton
  //---------------------------------------------------------------------------
  // Check Gen Level for ttbar sample
  if (TTbarMC_ >0){
    // Signal ttbar event
    if(TTbarMC_ == 1 && nGenLep != 1 && TTbarCatMC_ != 7) ch_tag = 999;
    // Background ttbar event
    if(TTbarMC_ == 2 && nGenLep == 1) ch_tag = 999;
    // ttbar Category
    if(!IsCat) ch_tag = 999;
  } // if(TTbarMC_ >0)

  if (ch_tag<2 && EvTrigger){ // Single lepton event

    b_Channel  = ch_tag;

    b_Lepton_pt  = lepton.Pt();
    b_Lepton_eta = lepton.Eta();
    b_Lepton_phi = lepton.Phi();
    b_Lepton_e   = lepton.E();
    b_Lepton_SF  = lepton_SF;
    b_Lepton_LES = lepton_LES;

    //---------------------------------------------------------------------------
    // Jets
    //---------------------------------------------------------------------------
    Handle<cat::JetCollection> jets;
    iEvent.getByToken(jetToken_, jets);

    // Arrange Jets by CSV discriminator
    std::vector<int>  JetIndex;
    for(unsigned int i = 0; i < jets->size() ; i++) JetIndex.push_back(i);
    /*
    for(unsigned int ijet=0; ijet < JetIndex.size(); ijet++){
      for(unsigned int jjet=ijet+1; jjet < JetIndex.size(); jjet++){
        const cat::Jet & jet_i = jets->at(JetIndex[ijet]);
        const cat::Jet & jet_j = jets->at(JetIndex[jjet]);

        float iJetdeepCSV = (jet_i.bDiscriminator(BTAG_DeepCSVb)+jet_i.bDiscriminator(BTAG_DeepCSVbb));
        float jJetdeepCSV = (jet_j.bDiscriminator(BTAG_DeepCSVb)+jet_j.bDiscriminator(BTAG_DeepCSVbb));

        if(jJetdeepCSV > iJetdeepCSV){
          float tempIndex = JetIndex[ijet];
          JetIndex[ijet] = JetIndex[jjet];
          JetIndex[jjet] = tempIndex;
        }//if(jJetCSV > iJetCSV)
      }
    }
    */

    int N_GoodJets = 0;

    // Initialize SF_btag
    // Jet_SF_CSV[Scenario][SystVariations];
    float Jet_SF_deepCSV[6][19];
    for (unsigned int ipTj=0; ipTj<5; ipTj++){
      for (unsigned int iu=0; iu<19; iu++) {
        Jet_SF_deepCSV[ipTj][iu] = 1.0;
      }
    }

    Handle<float> rSF_deepCSV;
    iEvent.getByToken(deepCSVWeightToken_, rSF_deepCSV);
    Jet_SF_deepCSV[4][0] = *rSF_deepCSV;

    Handle<std::vector<float>> rsysSF_deepCSV;
    iEvent.getByToken(deepCSVSysWeightToken_, rsysSF_deepCSV);
    for (unsigned int idcsv = 0; idcsv < rsysSF_deepCSV->size() ; idcsv++) Jet_SF_deepCSV[4][idcsv+1] = rsysSF_deepCSV->at(idcsv);

    std::vector<cat::Jet> selectedJets;

    // Run again over all Jets
    for (unsigned int i = 0; i < JetIndex.size() ; i++) {
      const cat::Jet & jet = jets->at(JetIndex[i]);

      bool goodJet  = false;
      bool cleanJet = false;

      // Jet Selection (pT>20GeV to take into account SYST Variations)
      if(std::abs(jet.eta()) < 2.4 && jet.pt() > 20. && jet.tightLepVetoJetID()) goodJet = true;
      // Jet Cleaning
      TLorentzVector vjet;
      vjet.SetPtEtaPhiE(jet.pt(), jet.eta(), jet.phi(), jet.energy());
      double dr_LepJet = vjet.DeltaR(lepton);
      if(dr_LepJet > 0.4) cleanJet = true;

      if(goodJet && cleanJet){
        selectedJets.push_back( jet );
        // Basic variables
        b_Jet_pt ->push_back(jet.pt());
        b_Jet_eta->push_back(jet.eta());
        b_Jet_phi->push_back(jet.phi());
        b_Jet_e  ->push_back(jet.energy());
        b_Jet_Index ->push_back(JetIndex[i]);
	
	      // Number of Jets (easy cross check)
        if(jet.pt() > 30.) N_GoodJets ++;

        // Parton Flavour
        b_Jet_partonFlavour->push_back(jet.partonFlavour());
        b_Jet_hadronFlavour->push_back(jet.hadronFlavour());

        // b-tag discriminant
        float jet_btagDis_CSV = jet.bDiscriminator(BTAG_CSVv2);
        b_Jet_CSV ->push_back(jet_btagDis_CSV);
        float jet_btagDis_deepCSVb = jet.bDiscriminator(BTAG_DeepCSVb);
        float jet_btagDis_deepCSVbb = jet.bDiscriminator(BTAG_DeepCSVbb);
        b_Jet_deepCSV ->push_back(jet_btagDis_deepCSVb+jet_btagDis_deepCSVbb);
        // c-tag discriminant
        float jet_btagDis_CvsL = jet.bDiscriminator(CTAG_CvsL);
        b_Jet_CvsL ->push_back(jet_btagDis_CvsL);
        float jet_btagDis_CvsB = jet.bDiscriminator(CTAG_CvsB);
	      b_Jet_CvsB ->push_back(jet_btagDis_CvsB);
        float jet_btagDis_deepCvsL = jet.bDiscriminator(CTAG_DeepCSVc)/(jet.bDiscriminator(CTAG_DeepCSVc)+jet.bDiscriminator(CTAG_DeepCSVudsg));
        b_Jet_deepCvsL ->push_back(jet_btagDis_deepCvsL);
        float jet_btagDis_deepCvsB = jet.bDiscriminator(CTAG_DeepCSVc)/(jet.bDiscriminator(CTAG_DeepCSVc)+jet.bDiscriminator(BTAG_DeepCSVb)+jet.bDiscriminator(BTAG_DeepCSVbb));
        b_Jet_deepCvsB ->push_back(jet_btagDis_deepCvsB);

        if(isMC_) {
          b_Jet_JES_Up  ->push_back(jet.shiftedEnUp());
          b_Jet_JES_Down->push_back(jet.shiftedEnDown());

          // Ref: https://twiki.cern.ch/twiki/bin/view/CMS/JetResolution
          b_Jet_JER_Up   ->push_back(jet.smearedResUp());
          b_Jet_JER_Nom  ->push_back(jet.smearedRes());
          b_Jet_JER_Down ->push_back(jet.smearedResDown());

          // Ref: https://twiki.cern.ch/twiki/bin/view/CMS/BTagShapeCalibration
          // Saving the central SF and the 18 syst. unc. for:
          if(jet.pt() > 25.) for (unsigned int iu=0; iu<19; iu++) Jet_SF_deepCSV[0][iu] *= SF_deepCSV_.getSF(jet, iu);
          if(jet.pt() > 30.) for (unsigned int iu=0; iu<19; iu++) Jet_SF_deepCSV[1][iu] *= SF_deepCSV_.getSF(jet, iu);
          if(jet.pt() > 35.) for (unsigned int iu=0; iu<19; iu++) Jet_SF_deepCSV[2][iu] *= SF_deepCSV_.getSF(jet, iu);
          if(jet.pt() > 40.) for (unsigned int iu=0; iu<19; iu++) Jet_SF_deepCSV[3][iu] *= SF_deepCSV_.getSF(jet, iu);
          // pT_Jets > 38 for first jet, and 30 for the others (e+jet)
          if(i == 0 and jet.pt() > 38.) for (unsigned int iu=0; iu<19; iu++) Jet_SF_deepCSV[4][iu] *= SF_deepCSV_.getSF(jet, iu);
          else if ( jet.pt() > 30.) for (unsigned int iu=0; iu<19; iu++) Jet_SF_deepCSV[4][iu] *= SF_deepCSV_.getSF(jet, iu);
        } // if(isMC_)	
      }// if(GoodJets)
    }// for(AllJets)
    
    // Number of Jets (easy cross check)
    b_Jet_Number = N_GoodJets;

    for (unsigned int iu=0; iu<19; iu++){
      b_Jet_SF_deepCSV_25->push_back(1.0);
      b_Jet_SF_deepCSV_30->push_back(1.0);
      b_Jet_SF_deepCSV_35->push_back(1.0);
      b_Jet_SF_deepCSV_40->push_back(1.0);
      b_Jet_SF_deepCSV_38->push_back(1.0);
      b_Jet_SF_deepCSV   ->push_back(1.0);
    }
    (*b_Jet_SF_deepCSV_25)[0] = Jet_SF_deepCSV[0][0]; //Central
    (*b_Jet_SF_deepCSV_30)[0] = Jet_SF_deepCSV[1][0]; //Central
    (*b_Jet_SF_deepCSV_35)[0] = Jet_SF_deepCSV[2][0]; //Central
    (*b_Jet_SF_deepCSV_40)[0] = Jet_SF_deepCSV[3][0]; //Central
    (*b_Jet_SF_deepCSV_38)[0] = Jet_SF_deepCSV[4][0]; //Central
    (*b_Jet_SF_deepCSV   )[0] = Jet_SF_deepCSV[5][0]; //Central
    // To save only the error
    for (unsigned int iu=1; iu<19; iu++){
      (*b_Jet_SF_deepCSV_25)[iu] = std::abs(Jet_SF_deepCSV[0][iu] - Jet_SF_deepCSV[0][0]); // Syst. Unc.
      (*b_Jet_SF_deepCSV_30)[iu] = std::abs(Jet_SF_deepCSV[1][iu] - Jet_SF_deepCSV[1][0]); // Syst. Unc.
      (*b_Jet_SF_deepCSV_35)[iu] = std::abs(Jet_SF_deepCSV[2][iu] - Jet_SF_deepCSV[2][0]); // Syst. Unc.
      (*b_Jet_SF_deepCSV_40)[iu] = std::abs(Jet_SF_deepCSV[3][iu] - Jet_SF_deepCSV[3][0]); // Syst. Unc.
      (*b_Jet_SF_deepCSV_38)[iu] = std::abs(Jet_SF_deepCSV[4][iu] - Jet_SF_deepCSV[4][0]); // Syst. Unc.
      (*b_Jet_SF_deepCSV   )[iu] = std::abs(Jet_SF_deepCSV[5][iu] - Jet_SF_deepCSV[5][0]); // Syst. Unc.
    }
   
    tree->Fill();
        
  }// if(ch_tag)
}

//------------- Good Muon Selection -----------------------
bool fcncLepJetsAnalyzer::IsSelectMuon(const cat::Muon & i_muon_candidate)
{
  bool GoodMuon=true;

  // Tight selection already defined into CAT::Muon
  GoodMuon &= (i_muon_candidate.isTightMuon());

  GoodMuon &= (i_muon_candidate.isPFMuon());           // PF
  GoodMuon &= (i_muon_candidate.pt()> 20);             // pT
  GoodMuon &= (std::abs(i_muon_candidate.eta())< 2.1); // eta

  //------------- The Relative Isolation is already calculated in the CAT object -----------------------
  // relIso( R ) already includes PU subtraction
  // float relIso = ( chIso + std::max(0.0, nhIso + phIso - 0.5*PUIso) )/ ecalpt;
  if ( !doLooseLepton_ ) GoodMuon &=( i_muon_candidate.relIso( 0.4 ) < 0.15 );

  return GoodMuon;
}
//------------- Loose Muon Selection -----------------------
bool fcncLepJetsAnalyzer::IsVetoMuon(const cat::Muon & i_muon_candidate){

  bool GoodMuon=true;

  // Loose selection already defined into CAT::Muon
  GoodMuon &= (i_muon_candidate.isLooseMuon());

  GoodMuon &= (i_muon_candidate.isPFMuon());           // PF
  GoodMuon &= (i_muon_candidate.pt()> 15);             // pT
  GoodMuon &= (std::abs(i_muon_candidate.eta())< 2.4); // eta

  //------------- The Relative Isolation is already calculated in the CAT object -----------------------
  // relIso( R ) already includes PU subtraction
  // float relIso = ( chIso + std::max(0.0, nhIso + phIso - 0.5*PUIso) )/ ecalpt;

  GoodMuon &=( i_muon_candidate.relIso( 0.4 ) < 0.25 );

  return GoodMuon;
}

//------------- Good Electron Selection -----------------------
bool fcncLepJetsAnalyzer::IsSelectElectron(const cat::Electron & i_electron_candidate){

  bool GoodElectron=true;

  GoodElectron &= (i_electron_candidate.isPF() );                // PF
  GoodElectron &= (i_electron_candidate.pt() > 20);              // pT
  GoodElectron &= (std::abs(i_electron_candidate.eta()) < 2.1);  // eta
  GoodElectron &= (std::abs(i_electron_candidate.scEta()) < 1.4442 || // eta Super-Cluster
                   std::abs(i_electron_candidate.scEta()) > 1.566);

  // Electron cut based selection, wp80 is tight
  // https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun2#Recommended_MVA_Recipe_for_regul
  if ( !doLooseLepton_ ) GoodElectron &= i_electron_candidate.electronID("mvaEleID-Fall17-iso-V1-wp80") > 0.0;
  else                   GoodElectron &= i_electron_candidate.electronID("mvaEleID-Fall17-noIso-V1-wp80") > 0.0;

  //------------- The Relative Isolation is already calculated in the CAT object -----------------------
  // relIso( R ) already includes AEff and RhoIso
  // float relIso = ( chIso + std::max(0.0, nhIso + phIso - rhoIso*AEff) )/ ecalpt;

  // Isolation is already included in the cut-based cuts, it is not needed 
  // if ( !doLooseLepton_ ) {
  //   if ( std::abs(i_electron_candidate.scEta()) <= 1.479)   GoodElectron &=( i_electron_candidate.relIso( 0.3 ) < 0.0695 );
  //   else GoodElectron &= ( i_electron_candidate.relIso( 0.3 ) < 0.0821 );
  // }
  //----------------------------------------------------------------------------------------------------

  return GoodElectron;

}
//------------- Loose Electron Selection -----------------------
bool fcncLepJetsAnalyzer::IsVetoElectron(const cat::Electron & i_electron_candidate){

  bool GoodElectron=true;

  GoodElectron &= (i_electron_candidate.isPF() );                // PF
  GoodElectron &= (i_electron_candidate.pt() > 15);              // pT
  GoodElectron &= (std::abs(i_electron_candidate.eta()) < 2.4);  // eta
  GoodElectron &= (std::abs(i_electron_candidate.scEta()) < 1.4442 || // eta Super-Cluster
                   std::abs(i_electron_candidate.scEta()) > 1.566);

  // https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun2#Recommended_MVA_Recipe_for_regul
  // the Iso ID has a better fake rejection at the working point with the same name.
  GoodElectron &= i_electron_candidate.electronID("mvaEleID-Fall17-iso-V1-wpLoose") > 0.0;

  //------------- The Relative Isolation is already calculated in the CAT object -----------------------
  // if ( std::abs(i_electron_candidate.scEta()) <= 1.479) GoodElectron &= ( i_electron_candidate.relIso( 0.3 ) < 0.175 );
  // else GoodElectron &= ( i_electron_candidate.relIso( 0.3 ) < 0.159 );
  //----------------------------------------------------------------------------------------------------

  return GoodElectron;
}

//define this as a plug-in
DEFINE_FWK_MODULE(fcncLepJetsAnalyzer);
