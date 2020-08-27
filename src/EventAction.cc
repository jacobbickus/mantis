#include "EventAction.hh"
#include "PMTHit.hh"
#include "DetectorConstruction.hh"
#include "StackingAction.hh"
#include "SteppingAction.hh"
#include "HistoManager.hh"

#include "G4EventManager.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4VVisManager.hh"
#include "G4ios.hh"
#include "G4UImanager.hh"
#include "G4SystemOfUnits.hh"

#include <vector>
#include "G4ParticleGun.hh"
#include <iostream>

extern G4String output_name;

EventAction::EventAction(G4ParticleGun* particle_gun, SteppingAction* stepA,
  const DetectorConstruction* det)
: fDetector(det),fPMTCollID(-1),fPMTThreshold(1),fForcedrawphotons(false),
fForcenophotons(false)
{
    particle_gun_local = particle_gun;
    stepA_local = stepA;
    fHitCount = 0;
    fPMTsAboveThreshold = 0;
}

EventAction::~EventAction()
{}


void EventAction::BeginOfEventAction(const G4Event* anEvent)
{
    E_beam=particle_gun_local->GetParticleEnergy(); // We get the energy of the particle at the onset of the event
    EventID = anEvent->GetEventID();
    ResetEverything(); //reset all the class variables

    fHitCount = 0;
    fPMTsAboveThreshold = 0;

    G4SDManager* SDman = G4SDManager::GetSDMpointer();

    if(fPMTCollID<0)
      fPMTCollID=SDman->GetCollectionID("pmtHitCollection");

}


void EventAction::EndOfEventAction(const G4Event* anEvent)
{
    std::string data_file_name;
    G4AnalysisManager* manager = G4AnalysisManager::Instance();

    manager->FillNtupleDColumn(3,0,E_beam);
    manager->FillNtupleDColumn(3,1,stepA_local->Ev.size());
    manager->AddNtupleRow(3);
    // For Data file

    if(output_name.compare(0,4,"none")!=0){
        data_file_name = output_name;
        data_file.open(data_file_name.c_str(),std::ios_base::app);

        for(int i=0;i<int(stepA_local->Ev.size());++i){ //looping over all the tracks in the water

            if(stepA_local->detector_hit.at(i)){
                E=stepA_local->Ev.at(i);    //fill the incident energy variable ONLY for incident hit entries
                //Edep=stepA_local->Edepv.at(i);
                x=stepA_local->xv.at(i);
                y=stepA_local->yv.at(i);
                z=stepA_local->zv.at(i);
                theta=stepA_local->thetav.at(i);
                ID=stepA_local->IDv.at(i);
                //TrackID=stepA_local->TrackIDv.at(i);
                CreatorProcessName=stepA_local->ProcessNamev.at(i); //copy over the name to the placeholder
                ParticleName=stepA_local->ParticleNamev.at(i); //same here
                Time=stepA_local->Timev.at(i);
                //IsSurfaceHitTrack=stepA_local->IsSurfaceHit.at(i);

            } // added by jake

            if(stepA_local->detector_hit.at(i) && SaveTrackInfo){
                if(output_name.compare(0,4,"none")!=0){
                    if(briefoutput){
                            data_file << std::setprecision(5);
                            data_file << E_beam
                            << "\t" << E
                            << "\t" << EventID
                            << "\t" << ParticleName.c_str()
                            << "\t" << CreatorProcessName.c_str()
                            << "\t" << Time
                            <<std::endl;

                    } // if briefoutput
                        else{ // if not briefoutput
                          data_file << std::setprecision(5);
                          data_file << E_beam
                                << "\t" << E
                                << "\t" << x
                                << "\t" << y
                                << "\t" << z
                                << "\t" << theta
                                << "\t" << Time
                                << "\t" << EventID
                                << "\t" << ParticleName.c_str()
                                << "\t" << CreatorProcessName.c_str()
                                <<std::endl;

                        } // else statement
                }// output name compare if statment

            }
        } // for loop
        data_file.close();
    }// if statement


    // End of Event PMT analysis

    PMTHitsCollection* pmtHC = nullptr;
    G4HCofThisEvent* hitsCE = anEvent->GetHCofThisEvent();

    // Get the hit collections
    if(hitsCE){
      if(fPMTCollID>=0) {
        pmtHC = (PMTHitsCollection*)(hitsCE->GetHC(fPMTCollID));
      }
    }

    if(pmtHC){
        G4ThreeVector reconPos(0.,0.,0.);
        G4int pmts=pmtHC->entries();
    //Gather info from all PMTs
        for(G4int i=0;i<pmts;i++){
            fHitCount += (*pmtHC)[i]->GetPhotonCount();
            reconPos+=(*pmtHC)[i]->GetPMTPos()*(*pmtHC)[i]->GetPhotonCount();
            if((*pmtHC)[i]->GetPhotonCount()>=fPMTThreshold){
                fPMTsAboveThreshold++;
            }
            else{//wasnt above the threshold, turn it back off
                (*pmtHC)[i]->SetDrawit(false);
            }
        }
        if(fHitCount > 0) {//dont bother unless there were hits
            reconPos/=fHitCount;
            fReconPos = reconPos;
          }
          pmtHC->DrawAllHits();
        }


    // Update Run Statistics


    // show event number for user

    std::cout << "\r\tEvent and tracks:\t " << anEvent->GetEventID() << "\t" << stepA_local->Ev.size() <<std::flush;
}


/*
 void EventAction::Weighting(const G4Event *anEvent){
    // should allow messenger to set this
    G4bool ApplyQEWeight = false;
    if(ApplyQEWeight){
        // Apply Weight to particles based on quantum efficiency
        // should allow messenger to set this
        std::vector<G4double> QE = {8.2667, 17.7143, 27.90, 34.4444, 39.68, 39.4545, 37.20, 28.6154, 5.3143}; // default for GaAsP(Cs) PMT
        // set Wavelength Threshold (nm)
        std::vector<G4double> wavelengths = {300,350,400,450,500,550,600,650,700};
    }
}
*/

void EventAction::ResetEverything(){

    E=-1;
    x=-1e5;
    y=-1e5;
    z=-1e5;
    theta=-1e5;
    ID=-100000;
    TrackID=-10;
    ProcID=-1;
    Time=-1;
    IsSurfaceHitTrack=false;
    stepA_local->Ev.clear();
    stepA_local->Edepv.clear();
    stepA_local->xv.clear();
    stepA_local->yv.clear();
    stepA_local->zv.clear();
    stepA_local->thetav.clear();
    stepA_local->IDv.clear();
    stepA_local->ParticleNamev.clear();
    ParticleName="n/a";
    stepA_local->ProcessNamev.clear();
    CreatorProcessName="n/a";
    stepA_local->TrackIDv.clear();
    stepA_local->EventIDv.clear();
    stepA_local->ProcIDv.clear();
    stepA_local->Timev.clear();
    stepA_local->detector_hit.clear();
    stepA_local->IsSurfaceHit.clear();
}
