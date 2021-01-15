// Always include
#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "DetectorConstruction.hh"
#include "physicsList.hh"
#include "ActionInitialization.hh"
// Typcially include
#include "time.h"
#include "Randomize.hh"
#include "vector"
#include <iostream>
#include "G4Types.hh"

#ifdef G4VIS_USE
#include "G4VisExecutive.hh"
#include "G4TrajectoryDrawByParticleID.hh"
#endif

#include "G4UIExecutive.hh"

// For G4cout and G4cerr handling
#include "MySession.hh"
#include "G4ios.hh"
#include "G4UIsession.hh"

// declare global variables
G4long seed;
G4String macro, root_output_name, gOutName, bremTest, standalone_in, verbose_in; 

namespace
{
void PrintUsage()
{
        std::cerr << "Usage: " << std::endl;
        std::cerr << "mantis [-m macro=mantis.in] [-s seed=1] [-o output_name] [-t bremTest=false] [-p standalone=false] [-v NRF_Verbose=false]" << std::endl;
}
}

int main(int argc,char **argv)
{
  // Defaults
  G4int start_time = time(0);
  G4bool use_xsec_tables = true;
  G4bool use_xsec_integration = true;
  G4bool force_isotropic = false;
  G4bool standalone = false;
  G4bool NRF_Verbose = false;
  G4bool addNRF = true;
  macro = "mantis.in";
  seed = 1;
  output = false;
  bremTest = "false";
  G4bool brem = false;

  // Detect interactive mode (if no arguments) and define UI session
  //
        G4UIExecutive* ui = 0;

        if ( argc == 1 ) {
          ui = new G4UIExecutive(argc, argv);
        }

        // Evaluate Arguments
        if ( argc > 13 )
        {
                PrintUsage();
                return 1;
        }

        for (G4int i=1; i<argc; i=i+2) {
                if (G4String(argv[i]) == "-m") macro = argv[i+1];
                else if (G4String(argv[i]) == "-s") seed = atoi(argv[i+1]);
                else if (G4String(argv[i]) == "-o") root_output_name = argv[i+1];
                else if (G4String(argv[i]) == "-t") bremTest = argv[i+1];
                else if (G4String(argv[i]) == "-p") standalone_in = argv[i+1];
                else if (G4String(argv[i]) == "-v") verbose_in = argv[i+1];
                else
                {
                        PrintUsage();
                        return 1;
                }
        }

        if(standalone_in == "True" || standalone_in == "true")
        {
          std::cout << "Standalone File Requested." << std::endl;
          standalone = true;
        }
        if(verbose_in == "True" || verbose_in == "true")
        {
          std::cout << "NRF Verbose set to: " << verbose_in << std::endl;
          NRF_Verbose = true;
        }
        std::string RootOutputFile = (std::string)root_output_name;
        if(RootOutputFile.find(".root")<RootOutputFile.length()) {
                gOutName=(std::string)RootOutputFile.substr(0, RootOutputFile.find(".root"));
        }
        else gOutName=(std::string)root_output_name;

        G4UImanager* UI = G4UImanager::GetUIpointer();
        MySession* LoggedSession = new MySession;
        if(! ui && macro != "vis_save.mac")
        {
          output = true;
          UI->SetCoutDestination(LoggedSession);
        }
        // choose the Random engine
        CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine);
        CLHEP::HepRandom::setTheSeed(seed);
        std::cout << "Seed set to: " << seed << std::endl;

        // construct the default run manager
        G4RunManager* runManager = new G4RunManager;
        if(bremTest == "True" || bremTest == "true")
        {
           std::cout << "Conducting Brem Test!" << std::endl;
           brem = true;
        }
        // set mandatory initialization classes
        DetectorConstruction* det = new DetectorConstruction(brem);
        runManager->SetUserInitialization(det);

        // Set up Physics List
        physicsList *thePL = new physicsList(addNRF, use_xsec_tables, use_xsec_integration, force_isotropic, standalone, NRF_Verbose);
        runManager->SetUserInitialization(thePL);
        runManager->SetUserInitialization(new ActionInitialization(det, brem, output));

        // Run manager initialized in macros
#ifdef G4VIS_USE
        G4VisManager* visManager = new G4VisExecutive();
        visManager->Initialize();

        G4TrajectoryDrawByParticleID *colorModel = new G4TrajectoryDrawByParticleID;
        colorModel->Set("neutron", "cyan");
        colorModel->Set("gamma", "green");
        colorModel->Set("e-", "red");
        colorModel->Set("e+", "blue");
        colorModel->Set("proton", "yellow");
        colorModel->SetDefault("gray");
        visManager->RegisterModel(colorModel);
        visManager->SelectTrajectoryModel(colorModel->Name());
#endif

if(! ui)
{
  G4String command = "/control/execute ";
  UI->ApplyCommand(command+macro);
}
else
{
  // interactive mode
  UI->ApplyCommand("/control/execute init_vis.mac");
  ui->SessionStart();
  delete ui;
}

if(ui || macro == "vis_save.mac")
{
  delete visManager;
}

  G4int stop_time = time(0);
  G4cout << "The MC took:\t\t" << stop_time - start_time << "s" <<G4endl;

  delete LoggedSession;
  delete runManager;

  std::cout<<" The MC took:\t\t" << stop_time - start_time <<"s"<< std::endl;
  std::cout << std::endl << " Run completed!"<< std::endl;

  return 0;
}
