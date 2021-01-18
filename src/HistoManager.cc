#include "HistoManager.hh"

extern G4String gOutName;
HistoManager::HistoManager(): fFactoryOn(false), chosen_energy(-1)
{
}

HistoManager::~HistoManager()
{
}

void HistoManager::Book(G4bool bremTest)
{
  G4AnalysisManager* manager = G4AnalysisManager::Instance();
  G4int nbins = 100000;
  manager->SetVerboseLevel(0);
  manager->SetNtupleMerging(true);
  xmax = chosen_energy;
  #if defined (G4ANALYSIS_USE_ROOT)
      if(!bremTest && chosen_energy < 0)
      {
          TFile *fin = TFile::Open("brems_distributions.root");
          hBrems  = (TH1D*) fin->Get("hBrems");
          if (hBrems)
          {
              G4cout << "Imported brems from " << fin->GetName() << G4endl << G4endl;
              xmax = hBrems->GetXaxis()->GetXmax();
              std::cout << "Found Input Max Energy: " << xmax << " MeV" << std::endl;
              G4cout << "Found Input Max Energy: " << xmax << " MeV" << G4endl;
              fin->Close();
          }
          else
          {
                  std::cerr << "Error reading from file " << fin->GetName() << std::endl;
                  exit(1);
          }
      }

  #endif

  // open output file
  G4bool fileOpen = manager->OpenFile(gOutName);

  if(! fileOpen){
    G4cerr << "HistoManager::Book(): Cannot Open " <<manager->GetFileName()<<G4endl;
    return;
  }
  else
  {
    G4cout << "HistoManager::Book() Opened!" << G4endl;
  }
  if(!bremTest)
  {
      // Create 0 1DHistogram for Interogation Object
      manager->CreateH1("IncObj","Incident Interrogation Obj", nbins, 0., xmax, "MeV");

      // Create 1,2,3 1D Histogram for incident water data
      manager->CreateH1("NRFIncWater","NRF Incident Water", nbins, 0., xmax, "MeV");
      manager->CreateH1("IncWaterLow","Incident Water Low Energy Spectrum", nbins, 0.,1E-5,"MeV");
      manager->CreateH1("IncWaterHigh", "Incident Water High Energy Spectrum", nbins,1E-5, xmax, "MeV");
    
      // Create 0 Ntuple for Brem Test Chop Data 
      manager->CreateNtuple("ChopperData", "Chopper Ntuple Data");
      manager->CreateNtupleDColumn("ChopperData");
      manager->FinishNtuple();
      // Create 1 Ntuple for cherenkov in water 
      manager->CreateNtuple("Cherenkov","Cherenkov Data");
      manager->CreateNtupleDColumn("BeamEnergy");
      manager->CreateNtupleIColumn("NumPhotons");
      manager->FinishNtuple();

      // Create 4 and 5 histogram for incident Photocathode
      manager->CreateH1("IncDetLow","Low Energy Photons Incident Photocathode",nbins,0.,1E-5,"MeV");
      manager->CreateH1("IncDetHigh", "High Energy Photons Incident Photocathode",nbins,1E-5, xmax, "MeV");

      // Create 6 Histogram for Energy if detected
      manager->CreateH1("Detected","Detected Spectrum", nbins, 0., 1E-5, "MeV");

      // Create 2 Ntuple for Reactions within detector
      manager->CreateNtuple("DetPro","Detector Processes");
      manager->CreateNtupleSColumn("Process");
      manager->FinishNtuple();
  }
  else
  {
      // Create 0 Ntuple for Brem Test Chop Data 
      manager->CreateNtuple("ChopperData", "Chopper Ntuple Data");
      manager->CreateNtupleDColumn("ChopperData");
      manager->FinishNtuple();
  }

  fFactoryOn = true;

}

void HistoManager::finish()
{
    if(! fFactoryOn){
        G4cout << "ERROR HistoManager::finish: Failed to write to file" << G4endl;
        return;
    }
    G4AnalysisManager* manager = G4AnalysisManager::Instance();
    manager->Write();
    manager->CloseFile();
    G4cout << "Ntuples are saved. " << G4endl;
    delete manager;
    fFactoryOn = false;
}
