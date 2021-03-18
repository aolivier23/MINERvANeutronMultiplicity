//PlotUtils includes
#include "PlotUtils/MnvH2D.h"

//ROOT includes
#include "TFile.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TStyle.h"
#include "TROOT.h"

//c++ includes
#include <iostream>

namespace
{
  const auto oneDFileName = "MuonPT_manyCandsMC.root";
  const auto twoDFileName = "MuonEfficiencyStudyMC.root";

  const int colors[] = {kBlack, kBlue, kRed, kGreen+2, kMagenta};

  const auto MECName = "Tracker_Efficiency_Numerator_2p2h";
  const std::vector<std::string> bkgNames = {"Tracker_Efficiency_Numerator_RES",
                                             "Tracker_Efficiency_Numerator_QE",
                                             "Tracker_Efficiency_Numerator_DIS",
                                             "Tracker_Efficiency_Numerator_Other"};
}

int plotEfficiencyAndProcesses()
{
  gStyle->SetHistLineWidth(3);
  gStyle->SetOptStat(0);
  gROOT->ForceStyle(); //Histograms I'm drawing were created with a different style,
                       //so I need to tell them to override that style with this one.

  auto oneDFile = TFile::Open(oneDFileName);
  if(!oneDFile)
  {
    std::cerr << "Failed to open " << oneDFileName << " in plotEfficiency.\n";
    return 1;
  }

  //1D efficiency
  auto oneDNum = static_cast<PlotUtils::MnvH1D*>(oneDFile->Get("Tracker_MuonPTSignal_EfficiencyNumerator"));
  oneDNum->Divide(oneDNum, static_cast<PlotUtils::MnvH1D*>(oneDFile->Get("Tracker_MuonPTSignal_EfficiencyDenominator")));
  oneDNum->Draw("HIST");
  gPad->Print("efficiency.png");

  auto twoDFile = TFile::Open(twoDFileName);
  if(!twoDFile)
  {
    std::cerr << "Failed to open " << twoDFileName << " in plotEfficiency.\n";
    return 2;
  }

  //2D efficiency
  //TODO: I could use "bkg" for the numerator below.
  /*auto twoDNum = static_cast<PlotUtils::MnvH2D*>(twoDFile->Get(""));
  twoDNum->Divide(twoDNum, static_cast<PlotUtils::MnvH2D*>(twoDFile->Get("")));
  twoDNum->Draw("colz");
  gPad->Print("efficiency2D.png");*/

  auto sum = static_cast<PlotUtils::MnvH2D*>(twoDFile->Get(bkgNames.front().c_str()));

  //Process breakdown by individual components
  int whichColor = 0;
  auto MECHist = static_cast<PlotUtils::MnvH2D*>(twoDFile->Get(MECName));
  MECHist->SetLineColor(colors[whichColor++]);
  MECHist->ProjectionY()->Draw("HIST");

  sum->SetLineColor(colors[whichColor++]);
  sum->ProjectionY()->Draw("HIST SAME");

  for(auto whichBkg = bkgNames.begin()+1; whichBkg != bkgNames.end(); ++whichBkg)
  {
    auto bkg = static_cast<PlotUtils::MnvH2D*>(twoDFile->Get(whichBkg->c_str()));
    bkg->SetLineColor(colors[whichColor++]);
    bkg->ProjectionY()->Draw("HIST SAME");
    sum->Add(bkg);
  }
  gPad->BuildLegend(0.7, 0.6, 0.95, 0.9);
  gPad->Print("processBreakdown.png");

  //Process breakdown with all backgrounds together
  MECHist->ProjectionY()->Draw("HIST");
  sum->SetLineColor(kRed);
  sum->SetTitle("All Others Stacked");
  auto projY = sum->ProjectionY();
  projY->SetTitle("All Others Stacked");
  projY->Draw("HIST SAME");
  gPad->BuildLegend(0.55, 0.6, 0.95, 0.9);
  gPad->Print("processBreakdownStacked.png");

  return 0;
}
