//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Author:
// Jacob E Bickus, 2021
// MIT, NSE
// jbickus@mit.edu
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
///////////////////////////////////////////////////////////////////////////////

#include "HistoManager.hh"

extern G4String gOutName;
extern G4String inFile;
extern G4double chosen_energy;
extern G4bool bremTest;

HistoManager::HistoManager() : fFactoryOn(false)
{
}

HistoManager::~HistoManager()
{
}

void HistoManager::Book()
{
        G4AnalysisManager* manager = G4AnalysisManager::Instance(); 
        manager->SetVerboseLevel(0);
        xmax = chosen_energy;
        
        if(!bremTest && chosen_energy < 0)
        {
                if(gSystem->AccessPathName(inFile.c_str()) == 0)
                {
                        TFile *fin = TFile::Open(inFile.c_str());
                        if(inFile.compare(0,24, "brems_distributions.root") == 0)
                                hBrems  = (TH1D*) fin->Get("hBrems");
                        else
                                hBrems = (TH1D*) fin->Get("ChopperIn_Weighted");
                        
                        if (!hBrems)
                        {
                                G4cerr << "HistoManager::Error reading from file " << fin->GetName() << G4endl;
                                exit(1);
                        }
                        else
                        {
                                xmax = hBrems->GetXaxis()->GetXmax();
                                G4cout << "Found Input Max Energy: " << xmax << " MeV" << G4endl;
                                fin->Close();
                        } // for if !hBrems            
                } // for if gSystem
                else
                {
                        G4cerr << "FATAL ERROR: HistoManager:: " << inFile << " not Found!" << G4endl;
                        exit(1);
                } // for if !gSystem 
        } // for if not bremTest and chosen_energy < 0

        // open output file
        G4bool fileOpen = manager->OpenFile(gOutName);
        
        //std::cout << "Energy for xMax: " << xmax << std::endl;
        
        if(!fileOpen)
        {
                G4cerr << "HistoManager::Book(): Cannot Open " <<manager->GetFileName()<<G4endl;
                return;
        }

        G4int nbins = xmax/(10.0e-6/2.0);
        // Create ID 0 Ntuple for Incident Chopper Data
        manager->CreateNtuple("ChopIn", "Chopper Wheel Incident Data");
        manager->CreateNtupleDColumn("Energy");
        manager->CreateNtupleDColumn("Weight");
        manager->CreateNtupleIColumn("EventID");
        manager->FinishNtuple();
        // Create ID 1 Ntuple for Exiting Chopper Data
        manager->CreateNtuple("ChopOut", "Chopper Wheel Exiting Radiation Data");
        manager->CreateNtupleDColumn("Energy");
        manager->CreateNtupleDColumn("Weight");
        manager->CreateNtupleIColumn("EventID");
        manager->CreateNtupleIColumn("isNRF");
        manager->FinishNtuple();

        // Create ID 0 1D Histogram for Weighted Chopper Incident Data
        manager->CreateH1("ChopperIn_Weighted","Weighted Incident Chopper Energy Spectrum", nbins, 0., xmax, "MeV");
        // Create ID 1 1D Histogram for Weighted Chopper Exiting Data
        manager->CreateH1("ChopperOut_Weighted", "Weighted Emission Chopper Energy Spectrum", nbins, 0., xmax, "MeV");

        if(!bremTest)
        {
                // Create ID 2,3,4,5 1D Histogram for Interogation Object Data
                manager->CreateH1("IntObjIn","Interrogation Object Incident Weighted Energy Spectrum", nbins, 0., xmax, "MeV");
                manager->CreateH1("NRFIntObjIn", "Interrogation Object NRF Photons Incident Weighted Energy Spectrum",nbins,0.,xmax, "MeV");
                manager->CreateH1("IntObjOut", "Interrogation Object Exiting Weighted Energy Spectrum", nbins, 0., xmax, "MeV");
                manager->CreateH1("NRFIntObjOut", "Interrogation Object NRF Photons Exiting Weighted Energy Spectrum", nbins, 0., xmax, "MeV");
                // Create ID 6,7 1D Histogram for incident water data
                manager->CreateH1("WaterIn", "Water Tank Incident Weighted Energy Spectrum", nbins,0., xmax, "MeV");
                manager->CreateH1("NRFWaterIn","Water Tank NRF Photons Incident Weighted Energy Spectrum", nbins, 0., xmax, "MeV");

                // Create ID 2 Ntuple for NRF Materials
                manager->CreateNtuple("NRFMatData","NRF Material vs Energy");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("Material");
                manager->CreateNtupleDColumn("ZPos");
                manager->FinishNtuple();

                // Create Histogram ID 8
                manager->CreateH1("NRF_Weighted","NRF Weighted Energy Spectrum", nbins, 0., xmax, "MeV");

                // Create ID 3 Ntuple for cherenkov in water
                manager->CreateNtuple("Cherenkov","Cherenkov in Water Data");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleIColumn("NumSecondaries");
                manager->CreateNtupleDColumn("Time");
                manager->FinishNtuple();

                // Create Histogram ID 9
                manager->CreateH1("Cherenkov_Weighted", "Cherenkov Weighted Energy Spectrum", nbins,0.,xmax,"MeV");

                // Create ID 4 Ntuple for Detected Information
                manager->CreateNtuple("DetInfo","Detected Information");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("CreatorProcess");
                manager->CreateNtupleDColumn("Time");
                manager->FinishNtuple();

                // Create ID 5 Ntuple for Detector Process Information
                manager->CreateNtuple("IncDetInfo","Incident Detector Process Information");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("DetProcess");
                manager->FinishNtuple();
                
                // Create ID 6 NTuple for Incident Interrogation Object Information
                // Added on later as a test 
                //manager->CreateNtuple("IncIntObjInfo","Incident Interrogation Object Information");
                //manager->CreateNtupleDColumn("Energy");
                //manager->CreateNtupleDColumn("Weight");
                //manager->CreateNtupleSColumn("CreatorProcess");
                //manager->CreateNtupleDColumn("Z_Position");
                //manager->CreateNtupleDColumn("Cos_theta");
                //manager->CreateNtupleDColumn("Time");
                //manager->FinishNtuple();

                // Create ID 10 Histogram for Incident Detector
                manager->CreateH1("Inc_Det_Weighted", "Incident Detector Weighted Energy Spectrum", nbins, 0., xmax, "MeV");

                // Create ID 11 Histogram for Energy if detected
                manager->CreateH1("Detected_Weighted","Photons Detected by Photocathode Weighted Energy Spectrum", nbins, 0., 100., "eV");

        }

        fFactoryOn = true;
        //std::cout << "HistoManager::Book() --> Complete!" << std::endl;

}

void HistoManager::finish()
{
        if(!fFactoryOn) {
                G4cout << "ERROR HistoManager::finish: Failed to write to file" << G4endl;
                return;
        }
        G4AnalysisManager* manager = G4AnalysisManager::Instance();
        manager->Write();
        manager->CloseFile();
        std::cout << "HistoManager::finish -> Ntuples are saved." << std::endl;
        G4cout << "HistoManager::finish -> Ntuples are saved. " << G4endl;
        delete manager;
        fFactoryOn = false;
}
