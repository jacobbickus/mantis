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

#ifndef RunAction_h
#define RunAction_h 1

#include "globals.hh"
#include "G4UserRunAction.hh"
#include <vector>
#include "G4Run.hh"
#include "HistoManager.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "EventCheck.hh"
#include "PrimaryGeneratorAction.hh"

class RunAction : public G4UserRunAction
{
  public:
    RunAction(HistoManager*, PrimaryGeneratorAction*);
    virtual ~RunAction();

  public:

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);

    void AddCerenkovEnergy(G4double en) {fCerenkovEnergy += en;}
    void AddScintillationEnergy(G4double en) {fScintEnergy += en;}
    void AddCerenkov(void) {fCerenkovCount++;} // changed from +=
    void AddScintillation(void) {fScintCount++;}
    void AddTotalSurface(void) {fTotalSurface += 1;}
    void AddNRF(void){fNRF++;}
    void AddStatusKilledPosition(void){fStatusKilledPosition++;}
    void AddStatusKilledTime(void){fStatusKilledTime++;}

  private:
    HistoManager* fHistoManager;
    PrimaryGeneratorAction* fpga;
    G4double fCerenkovEnergy, fScintEnergy, fCerenkovCount;
    G4int fScintCount, fTotalSurface, fNRF, fStatusKilledPosition, fStatusKilledTime;
};


#endif
