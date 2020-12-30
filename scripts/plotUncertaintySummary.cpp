//File: plotUncertaintySummary.cpp
//Brief: A ROOT script to plot an uncertainty summary for a number of neutrons distribution.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//c++ includes
#include <iostream>
#include <algorithm>
#include <memory>

//ROOT includes
#include "TFile.h"
#include "TCanvas.h"

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvPlotter.h"

namespace
{
  std::map<std::string, std::vector<std::string>> errorGroups = {{"CCQE Model", {"genie_CCQEPauliSupViaKF", "genie_NormCCQE", "genie_VecFFCCQEshape", "genie_MaCCQEshape"}},
                                                                 {"Nucleon FSI", {"genie_FrAbs_N", "genie_FrCEx_N", "genie_FrElas_N", "genie_FrInel_N", "genie_MFP_N"}},
                                                                 {"Pion FSI", {"genie_FrAbs_pi", "genie_FrCEx_pi", "genie_FrElas_pi", "genie_FrPiProd_pi", "genie_MFP_pi"}},
                                                                 {"genie_NormCCRES", {"genie_NormCCRES"}},
                                                                 {"Flux", {"Flux"}},
                                                                 {"RPA_LowQ2", {"RPA_LowQ2"}},
                                                                 {"2p2h", {"2p2h"}}};
  constexpr auto signalName = "Neutron_Multiplicity_SelectedMCEvents";
  constexpr auto bkgBaseName = "Neutron_Multiplicity_Background_";

  std::string replaceAll(std::string source, const char toReplace, const std::string& with)
  {
    for(size_t found = source.find(toReplace); found != std::string::npos; found = source.find(toReplace))
    {
      source.replace(found, 1, with);
    }

    return source;
  }
}

int plotUncertaintySummary(const std::string fileName)
{
  //Open the input file
  std::unique_ptr<TFile> inFile(TFile::Open(fileName.c_str(), "READ"));
  if(!inFile)
  {
    std::cerr << "Failed to open a ROOT file named " << fileName << ".\n";
    return 1;
  }

  const std::string baseName = fileName.substr(0, fileName.find(".root"));

  //Find the histogram of selected signal events
  auto signal = static_cast<PlotUtils::MnvH1D*>(inFile->Get(::signalName));
  if(!signal)
  {
    std::cerr << "Failed to find an MnvH1D called " << signalName << " in a file named " << fileName << ".\n";
    return 2;
  }

  THStack breakdown;
  breakdown.Add(static_cast<TH1D*>(signal->GetCVHistoWithError().Clone()));
  breakdown.SetHistogram(static_cast<TH1D*>(signal->GetCVHistoWithError().Clone()));

  //Add() all selected background events to signal
  for(auto key: *inFile->GetListOfKeys())
  {
    if(std::string(key->GetName()).find(bkgBaseName) != std::string::npos)
    {
      auto component = dynamic_cast<PlotUtils::MnvH1D*>(static_cast<TKey*>(key)->ReadObj());
      if(!component)
      {
        std::cerr << "An object named " << key->GetName() << " in " << fileName << " appeared to be related to the background name pattern "
                  << bkgBaseName << ", but it's not an MnvH1D!  Throw these results out.\n";
        return 3;
      }

      signal->Add(component);
      breakdown.Add(static_cast<TH1D*>(component->GetCVHistoWithError().Clone()));
    }
  }

  //Put unused error bands into the "Other" category
  const auto allBandNames = signal->GetVertErrorBandNames();
  auto& other = ::errorGroups["Other"];
  for(const auto& name: allBandNames)
  {
    if(std::find_if(::errorGroups.begin(), errorGroups.end(),
                    [&name](const auto& group)
                    { return std::find(group.second.begin(), group.second.end(), name) != group.second.end(); })
       == ::errorGroups.end())
    {
      other.push_back(name);
    }
  }

  //Set up a MnvPlotter
  TCanvas output(baseName.c_str(), "No Title");
  PlotUtils::MnvPlotter plotter;
  plotter.error_summary_group_map = ::errorGroups;

  //Plot the error band summary itself
  output.SetTitle("Error Band Summary");
  plotter.DrawErrorSummary(signal);
  output.Print((baseName + "_errors.png").c_str());

  //Plot the total signal with error bars
  output.SetTitle("Total Signal");
  auto total =  static_cast<TH1D*>(signal->GetCVHistoWithError().Clone());
  total->Draw();
  output.Print((baseName + "_totalSignal.png").c_str());

  //Plot a stack of selected events with error bars
  output.SetTitle("Background Breakdown");
  breakdown.Draw("HIST PFC PLC");
  output.BuildLegend(0.6, 0.65, 0.9, 0.95);
  output.Print((baseName + "_breakdown.png").c_str());

  //Plot each error band category
  for(const auto& cat: ::errorGroups)
  {
    output.SetTitle(cat.first.c_str());
    plotter.DrawErrorSummary(signal, "TR", true, true, 0.00001, false, cat.first);
    output.Print((baseName + "_" + ::replaceAll(cat.first, ' ', "_") + ".png").c_str());
  }

  return 0;
}
