//
// ************************************************************************************************ // 
// ************************************************************************************************ //
// To Run:
// root -b -q 'Analysis.cc("testOn","testOff", 2.1)'
// ************************************************************************************************ //
// ************************************************************************************************ //
// File Explanation:
// 
// Execution of this script in ROOT CERN executes 4 other scripts in ROOT CERN 
// 1. Cherenkov.cc
// 2. EventCheck.cc
// 3. WeightHisto.cc
// 4. PrintResults.cc
// 
// Analysis.cc takes 3 inputs:
// 1. Chopper On File root name (without .root extension)
// 2. Chopper Off File root name (without .root extension)
// 3. Max Energy of Bremsstrahlung Interrogation Beam 
//
// ************************************************************************************************ //
// ************************************************************************************************ //

#include "WeightHisto.cc"
#include "EventCheck.cc"
#include "Cherenkov.cc"
#include "PrintResults.cc"

void Analysis(const char* InputFilenameBaseOn, 
              const char* InputFilenameBaseOff, 
              double Emax)
{
    std::cout << "Merging Cherenkov, Checking Events, and Writing Weighted Histograms!" << std::endl;
    bool tcheck = false;
    // Chopper On
    Cherenkov(InputFilenameBaseOn, Emax, true);
    std::cout << std::endl << "On File Cherenkov Merged!" << std::endl;
    EventCheck(InputFilenameBaseOn, true);
    std::cout << std::endl << "On File Events Checked!" << std::endl << "Beginning to Weight Histograms..." << std::endl;
    bool checkOn = WeightHisto(InputFilenameBaseOn, Emax, true);
    std::cout << std::endl << "On File Added Weighted Histograms!" << std::endl;
    // Chopper Off
    Cherenkov(InputFilenameBaseOff, Emax, false);
    std::cout << std::endl << "Off File Cherenkov Merged!" << std::endl;
    EventCheck(InputFilenameBaseOff, false);
    std::cout << std::endl << "Off File Events Checked!" << std::endl << "Beginning to Weight Histograms..." << std::endl;
    bool checkOff = WeightHisto(InputFilenameBaseOff, Emax, false);
    std::cout << std::endl << "Off File Added Weighted Histograms!" << std::endl;
    if(checkOn && checkOff)
    {
      tcheck = true;
    }
    std::cout << std::endl << std::endl << "Printing Results..." << std::endl << std::endl;
    PrintResults(InputFilenameBaseOn, InputFilenameBaseOff, true, true, tcheck);
    std::cout << std::endl << "Analysis Complete" << std::endl << std::endl;
}
