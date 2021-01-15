#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "HistoManager.hh"
#include "globals.hh"
#include <vector>
#include "G4ParticleGun.hh"
#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "G4Gamma.hh"
#include "G4Electron.hh"
#include "eventInformation.hh"

#if defined (G4ANALYSIS_USE_ROOT)
#include "TFile.h"
#include "TROOT.h"
#include "TH1D.h"
#include "TRandom1.h"
#endif

class G4Event;
class PrimaryGenActionMessenger;
class HistoManager;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{

public:
PrimaryGeneratorAction(G4bool);
virtual ~PrimaryGeneratorAction();

public:
virtual void GeneratePrimaries(G4Event*);
G4ParticleGun* GetParticleGun(){
        return fParticleGun;
};
void SetEnergyValue(G4double val){
        chosen_energy = val;
}

private:
G4bool bremTest;
G4double chosen_energy;
PrimaryGenActionMessenger* genM;
G4ParticleGun* fParticleGun;
HistoManager* histo;
#if defined (G4ANALYSIS_USE_ROOT)
TRandom1 Random;
TH1D *hBrems;
TH1D *hSample;
#endif

protected:

int particleNumber;
G4double r;
G4double ph;
G4double x_r,y_r,z_r;
G4double z0;
G4double phi;
G4double theta;
G4bool randomizePrimary;
G4float beam_offset_x,beam_offset_y,beam_size,source_width;
G4float energy;
double random;
};


#endif
