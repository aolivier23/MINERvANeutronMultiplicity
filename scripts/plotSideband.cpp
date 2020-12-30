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
const double maxMC = 5e4; //Maximum across all plots I want to compare
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
    hist.SetLineWidth(lineSize);
    hist.SetFillColor(colors.at(whichHist)); //Use only without nostack (Yes, that's a double negative)
  }
}

int plotSideband(const std::string& dataFileName, const std::string& mcFileName)
{
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0); //I'll draw it myself
  gStyle->SetTitleSize(0.08, "pad");

  auto dataFile = giveMeFileOrGiveMeDeath(dataFileName),
       mcFile   = giveMeFileOrGiveMeDeath(mcFileName);

  const std::string fiducialName = "Tracker", sidebandName = "EAvailable",
                    dataName = fiducialName + "_" + sidebandName + "_Data";
  const std::regex find(fiducialName + "_" + sidebandName + R"(_(.*))");

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
  auto mcStack = makeStack(stackHists);
  auto dataHist = dynamic_cast<PlotUtils::MnvH1D*>(dataFile->Get(dataName.c_str()));
  if(!dataHist)
  {
    std::cerr << "Failed to find a histogram named " << dataName << "\n";
    return 1;
  }

  auto dataWithStatErr = static_cast<TH1D*>(dataHist->GetCVHistoWithError().Clone());

  auto errBandTemplate = mcFile->Get((fiducialName + "_" + sidebandName + "_TruthSignal").c_str());
  if(!errBandTemplate)
  {
    std::cerr << "Failed to find the signal histogram for error band " << sidebandName << "\n";
    return 1;
  }
  dataHist->AddMissingErrorBandsAndFillWithCV(*dynamic_cast<PlotUtils::MnvH1D*>(errBandTemplate));

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
  mcTotal->SetTitle("MnvGENIEv1");

  //mcTotal->GetYaxis()->SetTitle("candidates / event"); //dataWithStatErr->GetYaxis()->GetTitle()); //TODO: I had the axes backwards in the original plo
  mcTotal->GetYaxis()->SetLabelSize(labelSize * (bottomFraction + margin));
  mcTotal->GetYaxis()->SetTitleSize(0.06); //TODO: How to guess what these are?
  mcTotal->GetYaxis()->SetTitleOffset(0.6);

  //mcTotal->SetLineColor(kRed);
  //mcTotal->SetFillColorAlpha(kPink + 1, 0.4);
  mcTotal->SetMaximum(maxMC);
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
       mcRatio = std::accumulate(stackHists.begin()+1, stackHists.end(), static_cast<PlotUtils::MnvH1D*>(stackHists.front()->Clone()),
                                 [dataPOT, mcPOT](auto sum, const PlotUtils::MnvH1D* hist)
                                 {
                                   sum->Add(hist);
                                   return sum;
                                 });
  ratio->Divide(ratio, mcRatio);

  //Now fill mcRatio with 1 for bin content and fractional error
  for(int whichBin = 0; whichBin <= mcRatio->GetXaxis()->GetNbins(); ++whichBin)
  {
    mcRatio->SetBinError(whichBin, mcRatio->GetBinError(whichBin)/mcRatio->GetBinContent(whichBin));
    mcRatio->SetBinContent(whichBin, 1);
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
  //ratio->GetXaxis()->SetTitle("energy deposits [MeV]"); //dataWithStatErr->GetXaxis()->GetTitle()); //TODO: I had the axes backwards in the original plot
  ratio->GetXaxis()->SetLabelSize(labelSize);

  ratio->SetMinimum(minRatio);
  ratio->SetMaximum(maxRatio);
  ratio->Draw();

  mcRatio->SetLineColor(kRed);
  mcRatio->SetLineWidth(lineSize);
  mcRatio->SetFillColorAlpha(kPink + 1, 0.4);
  mcRatio->Draw("E2SAME");

  //Draw a flat line through the center of the MC
  auto straightLine = static_cast<TH1*>(mcRatio->Clone());
  straightLine->SetFillStyle(0);
  straightLine->Draw("HISTSAME");
  //TODO: Do uncertainty propagation correctly.  Looks like I'm assuming data and MC are uncorrelated for now which is roughly true.

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
