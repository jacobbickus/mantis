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

        G4int nbins = xmax/(10.0e-6); // this sets the bin width to 10 eV
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

        if(!bremTest)
        {

                // Create ID 2 Ntuple for NRF Materials
                manager->CreateNtuple("NRF","NRF Data");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("Material");
                manager->CreateNtupleDColumn("ZPos");
                manager->FinishNtuple();

                // Create ID 3 NTuple for Incident Interrogation Object Information
                manager->CreateNtuple("IntObjIn","Incident Interrogation Object Data");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("CreatorProcess");
                manager->FinishNtuple();

                // Create ID 4 Ntuple for IntObj Emission Data
                manager->CreateNtuple("IntObjOut","Interrogation Object Emission Data");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("CreatorProcess");
                manager->FinishNtuple();

                // Create ID 5 Ntuple for Incident Water Tank Data 
                manager->CreateNtuple("Water","Incident Water Tank Data");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("CreatorProcess");
                manager->FinishNtuple();

                // Create ID 6 Ntuple for cherenkov in water
                manager->CreateNtuple("Cherenkov","Cherenkov in Water Data");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleIColumn("NumSecondaries");
                manager->CreateNtupleDColumn("Time");
                manager->FinishNtuple();

                // Create ID 7 Ntuple for Detected Information
                manager->CreateNtuple("DetInfo","Detected Information");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("CreatorProcess");
                manager->CreateNtupleDColumn("Time");
                manager->FinishNtuple();

                // Create ID 8 Ntuple for Detector Process Information
                manager->CreateNtuple("IncDetInfo","Incident Detector Process Information");
                manager->CreateNtupleIColumn("EventID");
                manager->CreateNtupleDColumn("Energy");
                manager->CreateNtupleDColumn("Weight");
                manager->CreateNtupleSColumn("DetProcess");
                manager->FinishNtuple();

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
