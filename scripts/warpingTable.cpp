//File: warpingTable.cpp
//Brief: Prints warping study results for a table using a file produced by TransWarpExtractor.
//Author: Andrew Olivier aolivier@ur.rochester.edu

#include "TFile.h"
#include "TProfile.h"

#include <iostream>
#include <fstream>

int warpingTable(const std::string& fileName)
{
  auto file = TFile::Open(fileName.c_str(), "READ");

  const size_t lastSlash = fileName.rfind('/');
  const std::string outName = fileName.substr(lastSlash + 1, fileName.find('.') - lastSlash - 1);

  file->cd("Chi2_Iteration_Dists");
  auto chi2VsIterations = static_cast<TProfile*>(gDirectory->Get("m_avg_chi2_modelData_trueData_iter_chi2_truncated"));

  //Write interesting convergence statistics to a .csv file for a spreadsheet program to read later.
  std::ofstream outFile(outName + ".csv");
  outFile << outName << "," << chi2VsIterations->GetMinimum() << "," << chi2VsIterations->GetMinimumBin() << "\n";

  std::cout << outName << "," << chi2VsIterations->GetMinimum() << "," << chi2VsIterations->GetMinimumBin() << "\n";

  TCanvas can(outName.c_str());
  can.cd();
  chi2VsIterations->Draw();
  can.Print((outName + ".png").c_str());

  return 0;
}
