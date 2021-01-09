//File: warpingTable.cpp
//Brief: Prints warping study results for a table using a file produced by TransWarpExtractor.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#include "TFile.h"
#include "TProfile.h"

#include <iostream>
#include <fstream>
#include <sstream>

int warpingTable(const std::string& fileName)
{
  auto file = TFile::Open(fileName.c_str(), "READ");

  const size_t lastSlash = fileName.rfind('/');
  const std::string outName = fileName.substr(lastSlash + 1, fileName.find(".root") - lastSlash - 1);

  const std::string toSearchFor = "warpedMC";
  const std::string univName = outName.substr(outName.find(toSearchFor) + toSearchFor.length(), std::string::npos);

  file->cd("Chi2_Iteration_Dists");
  auto chi2VsIterations = static_cast<TProfile*>(gDirectory->Get("m_avg_chi2_modelData_trueData_iter_chi2_truncated"));

  //auto migrationMatrix = TODO;
  const int NDOF = 4; //migrationMatrix->GetXaxis()->GetNbins() - 2;

  //Write interesting convergence statistics to a .csv file for a spreadsheet program to read later.
  std::stringstream summary;
  summary << univName << "," << chi2VsIterations->GetMinimum() << "," << chi2VsIterations->GetBinCenter(chi2VsIterations->GetMinimumBin()) << "," << chi2VsIterations->GetBinCenter(chi2VsIterations->FindFirstBinAbove(NDOF)) << "," << chi2VsIterations->GetBinCenter(chi2VsIterations->FindFirstBinAbove(NDOF*5)) << "," << chi2VsIterations->GetBinCenter(chi2VsIterations->FindLastBinAbove(NDOF)) << "," << chi2VsIterations->GetBinCenter(chi2VsIterations->FindLastBinAbove(NDOF*5)) << std::endl;

  std::ofstream outFile(outName + ".csv");
  outFile << summary.str();
  std::cout << summary.str();

  TCanvas can(outName.c_str());
  can.cd();
  chi2VsIterations->Draw();
  can.Print((outName + ".png").c_str());

  return 0;
}
