//File: plotSideband.cpp
//Brief: Draws data and MC histograms on the same canvas with a ratio of Data/MC
//       on a canvas below for a sideband.
//Author: Andrew Olivier aolivier@ur.rochester.edu

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvColors.h"

//ROOT includes
#include "TFile.h"
#include "TKey.h"
#include "TParameter.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TLegend.h"
#include "TPaveText.h"

//c++ includes
#include <iostream>
#include <string>
#include <regex>
#include <numeric>

//I hate global variables, but it's after 10PM...
const int lineSize = 2;
const double maxMC = 1.5e4; //Maximum across all plots I want to compare
const double minRatio = 0.6, maxRatio = 1.2;

std::vector<PlotUtils::MnvH1D*> select(TFile& file, const std::regex& match, const double POTRatio)
{
  std::vector<PlotUtils::MnvH1D*> found;

  for(auto key: *file.GetListOfKeys())
  {
    if(std::regex_match(key->GetName(), match))
    {
      auto hist = dynamic_cast<PlotUtils::MnvH1D*>(static_cast<TKey*>(key)->ReadObj());
      if(hist)
      {
        hist->Scale(POTRatio);
        found.push_back(hist);
      }
    }
  }

  return found;
}

THStack makeStack(std::vector<PlotUtils::MnvH1D*>& hists)
{
  THStack stacked;
  for(auto hist: hists) stacked.Add(static_cast<TH1D*>(hist->GetCVHistoWithError().Clone()));
  return stacked;
}

TFile* giveMeFileOrGiveMeDeath(const std::string& fileName)
{
  auto file = TFile::Open(fileName.c_str());
  if(!file) throw std::runtime_error("Failed to open a file named " + fileName);
  return file;
}

void applyColors(TList& hists, const std::vector<int>& colors)
{
  for(int whichHist = 0; whichHist < hists.GetEntries(); ++whichHist)
  {
    auto& hist = dynamic_cast<TH1&>(*hists.At(whichHist));
    hist.SetLineColor(colors.at(whichHist));
    hist.SetLineWidth(0); //Took this out because line widths were making it look like top histogram has contributions when it has none.
    hist.SetFillColor(colors.at(whichHist)); //Use only without nostack (Yes, that's a double negative)
  }
}

int backgroundBreakdown(const std::string& dataFileName, const std::string& mcFileName, const std::string& sidebandName, const bool isSelected = false)
{
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0); //I'll draw it myself
  gStyle->SetTitleSize(0.08, "pad");

  auto dataFile = giveMeFileOrGiveMeDeath(dataFileName),
       mcFile   = giveMeFileOrGiveMeDeath(mcFileName);

  const std::string fiducialName = "Tracker",
                    dataName = fiducialName + "_" + sidebandName + "_" + (isSelected?"Signal":"Data"),
                    mcSignalName = fiducialName + "_"  + sidebandName + "_" + (isSelected?"SelectedMCEvents":"TruthSignal");
  const std::regex find(fiducialName + "_" + sidebandName + R"(_Background_(.*))");

  auto mcPOTInfo = dynamic_cast<TParameter<double>*>(mcFile->Get("POTUsed"));
  if(!mcPOTInfo)
  {
    std::cerr << mcFile->GetName() << " doesn't have POT information.\n";
    return 1;
  }
  const double mcPOT = mcPOTInfo->GetVal();

  auto dataPOTInfo = dynamic_cast<TParameter<double>*>(dataFile->Get("POTUsed"));
  if(!dataPOTInfo)
  {
    std::cerr << dataFile->GetName() << " doesn't have POT information.\n";
    return 1;
  }
  const double dataPOT = dataPOTInfo->GetVal();

  auto stackHists = select(*mcFile, find, dataPOT/mcPOT);

  auto mcSelected = dynamic_cast<PlotUtils::MnvH1D*>(mcFile->Get(mcSignalName.c_str()));
  if(!mcSelected)
  {
    std::cerr << "Failed to find a histogram named " << mcSignalName << " in " << mcFile->GetName() << "\n";
    return 1;
  }
  mcSelected->SetTitle("Signal");
  mcSelected->Scale(dataPOT/mcPOT);
  stackHists.push_back(mcSelected);
  auto mcStack = makeStack(stackHists);

  auto dataHist = dynamic_cast<PlotUtils::MnvH1D*>(dataFile->Get(dataName.c_str()));
  if(!dataHist)
  {
    std::cerr << "Failed to find a histogram named " << dataName << " in " << dataFile->GetName() << "\n";
    return 1;
  }

  auto dataWithStatErr = static_cast<TH1D*>(dataHist->GetCVHistoWithError().Clone());

  dataHist->AddMissingErrorBandsAndFillWithCV(*dynamic_cast<PlotUtils::MnvH1D*>(mcSelected));

  //Set histogram styles
  applyColors(*mcStack.GetHists(), MnvColors::GetColors(MnvColors::kOkabeItoDarkPalette));

  //Set up a Canvas split in 2.  The bottom canvas really overlaps the top by margin
  //to avoid drawing the x axis twice.
  TCanvas overall(("Data/MC for " + fiducialName + " " + sidebandName + " Sideband").c_str());

  //Turns out that when you create a TPad while there's a TCanvas,
  //the canvas automatically becomes the parent of that TPad.
  const double bottomFraction = 0.2, margin = 0.078; //margin was tuned by hand
  TPad top("Overlay", "Overlay", 0, bottomFraction, 1, 1),
       bottom("Ratio", "Ratio", 0, 0, 1, bottomFraction + margin);
  //Thou shalt Draw() new TPads lest they be blank!
  top.Draw();
  bottom.Draw();

  const double labelSize = 0.15;

  //Data with stacked MC
  top.cd();

  auto mcTotal = static_cast<TH1*>(mcStack.GetStack()->Last());
  mcTotal->SetTitle("MnvTunev1");

  mcTotal->GetYaxis()->SetLabelSize(labelSize * (bottomFraction + margin));
  mcTotal->GetYaxis()->SetTitleSize(0.06); //TODO: How to guess what these are?
  mcTotal->GetYaxis()->SetTitleOffset(0.6);

  //mcTotal->SetLineColor(kRed);
  //mcTotal->SetFillColorAlpha(kPink + 1, 0.4);
  //mcTotal->SetMaximum(maxMC);
  mcTotal->SetMaximum(2.*mcTotal->GetMaximum());
  mcTotal->SetMinimum(0);
  mcTotal->Draw("E2"); //Draw the error bars
  //Even though I throw the previous line's drawing away when I Draw("HIST") without SAME,
  //I still have to Draw() it to avoid a segmentation fault.  This is probably THStack
  //creating some histogram on demand.

  //mcStack.Draw("HISTnostackSAME");
  mcStack.Draw("HIST");
  auto axes = mcStack.GetHistogram(); //N.B.: GetHistogram() would have returned nullptr had I called it before Draw()!

  dataWithStatErr->SetLineColor(1);
  dataWithStatErr->SetLineWidth(lineSize);
  dataWithStatErr->SetMarkerStyle(20); //Resizeable closed circle
  dataWithStatErr->SetMarkerColor(1);
  dataWithStatErr->SetMarkerSize(0.7);
  dataWithStatErr->SetTitle("Data");
  dataWithStatErr->Draw("SAME");

  auto legend = top.BuildLegend(0.5, 0.4, 0.9, 0.9);

  //Drawing the thing that I don't want in the legend AFTER
  //building the legend.  What a dirty hack!
  auto lineOnly = static_cast<TH1*>(mcTotal->Clone());
  lineOnly->SetFillStyle(0);
  lineOnly->Draw("HISTSAME"); //Draw the line

  //Data/MC ratio panel
  bottom.cd();
  bottom.SetTopMargin(0);
  bottom.SetBottomMargin(0.3);
  auto ratio = static_cast<PlotUtils::MnvH1D*>(dataHist->Clone()),
       mcTotalWithSys = std::accumulate(stackHists.begin()+1, stackHists.end(), static_cast<PlotUtils::MnvH1D*>(stackHists.front()->Clone()),
                                        [dataPOT, mcPOT](auto sum, const PlotUtils::MnvH1D* hist)
                                        {
                                          sum->Add(hist);
                                          return sum;
                                        });
  ratio->Divide(ratio, mcTotalWithSys); //This is what MnvPlotter does too: Divide() MnvH1Ds directly.


  //Now fill mcRatio with 1 for bin content and fractional error
  auto mcRatio = mcTotalWithSys->GetTotalError(false, true, false); //The second "true" makes this fractional error.
  for(int whichBin = 0; whichBin <= mcRatio.GetXaxis()->GetNbins(); ++whichBin)
  {
    mcRatio.SetBinError(whichBin, std::max(mcRatio.GetBinContent(whichBin), 1e-9)); //TH1::Draw() behaves very badly when errors are exactly 0, so set them to a very small value instead.
    mcRatio.SetBinContent(whichBin, 1);
  }

  ratio->SetTitle("");
  ratio->SetLineWidth(lineSize);
  ratio->SetLineColor(kBlack);
  ratio->SetTitleSize(0);

  ratio->GetYaxis()->SetTitle("Data / MC");
  ratio->GetYaxis()->SetLabelSize(labelSize);
  ratio->GetYaxis()->SetTitleSize(0.16);
  ratio->GetYaxis()->SetTitleOffset(0.2);
  ratio->GetYaxis()->SetNdivisions(505); //5 minor divisions between 5 major divisions

  ratio->GetXaxis()->SetTitleSize(0.16);
  ratio->GetXaxis()->SetTitleOffset(0.9);
  ratio->GetXaxis()->SetLabelSize(labelSize);

  /*ratio->SetMinimum(minRatio);
  ratio->SetMaximum(maxRatio);*/
  ratio->Draw(); //Nota Bene: Only draws statistical error bars on the ratio.
                 //           The systematic errors are not part of the sumw2, and we do not override GetBinError().

  mcRatio.SetLineColor(kRed);
  mcRatio.SetLineWidth(lineSize);
  mcRatio.SetFillColorAlpha(kPink + 1, 0.4);
  mcRatio.Draw("E2 SAME");

  //Draw a flat line through the center of the MC
  auto straightLine = static_cast<TH1*>(mcRatio.Clone());
  straightLine->SetFillStyle(0);
  straightLine->Draw("HISTSAME");

  //Title for the whole plot
  top.cd();
  TPaveText title(0.3, 0.91, 0.7, 1.0, "nbNDC"); //no border
  title.SetFillStyle(0);
  title.SetLineColor(0);
  title.AddText("Tracker"); //TODO: Get this from the file name?
  title.Draw();

  //MINERvA Preliminary
  TPaveText prelim(0.12, 0.75, 0.47, 0.89, "nbNDC"); //no border
  prelim.SetFillStyle(0);
  prelim.SetLineColor(0);
  prelim.SetTextColor(kBlue);
  prelim.AddText("MINERvA Work in Progress"); //Preliminary");
  prelim.AddText("Stat. Errors Only");
  prelim.Draw();

  overall.Print((fiducialName + sidebandName + "DataMCRatio.png").c_str()); //TODO: Include file name here

  return 0;
}
