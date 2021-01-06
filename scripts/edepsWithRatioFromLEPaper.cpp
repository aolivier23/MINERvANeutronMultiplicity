//File: edepsWithRatioFromLEPaper.cpp
//Brief: Draws data and MC histograms on the same canvas with a ratio of Data/MC
//       on a canvas below.  Accepts any number of additional MC predictions and
//       and draws their ratios too.  This is my approximation to figure 5 of https://arxiv.org/pdf/1901.04892.pdf
//Author: Andrew Olivier aolivier@ur.rochester.edu

//PlotUtils includes
#include "PlotUtils/MnvH1D.h"
#include "PlotUtils/MnvColors.h"

//ROOT includes
#include "TFile.h"
#include "TKey.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TLegend.h"
#include "TPaveText.h"

//c++ includes
#include <iostream>
#include <string>
#include <regex>

//I hate global variables, but it's after 10PM...
const int lineSize = 2;
const double maxMC = 2; //Maximum across all plots I want to compare
const double minRatio = 0.5, maxRatio = 1.9;

THStack select(TFile& file, const std::regex& match)
{
  THStack found;

  for(auto key: *file.GetListOfKeys())
  {
    if(std::regex_match(key->GetName(), match))
    {
      auto hist = dynamic_cast<PlotUtils::MnvH1D*>(static_cast<TKey*>(key)->ReadObj());
      if(hist)
      {
        found.Add(static_cast<TH1D*>(hist->GetCVHistoWithError().Clone()));
      }
    }
  }

  return found;
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
    //hist.SetFillColor(colors.at(whichHist)); //Use only without nostack (Yes, that's a double negative)
  }
}

template <class ...ARGS>
int edepsWithRatioFromLEPaper(const std::string& dataFileName, const std::string& mcFileName, const ARGS&... otherMCFileNames)
{
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0); //I'll draw it myself
  gStyle->SetTitleSize(0.08, "pad");

  auto dataFile = giveMeFileOrGiveMeDeath(dataFileName),
       mcFile   = giveMeFileOrGiveMeDeath(mcFileName);
  //std::vector<TFile*> otherMCFiles = {giveMeFileOrGiveMeDeath(otherMCFileName)...};

  const std::string var = "EDeps", anaName = "Tracker_Neutron_Detection",
                    dataName = anaName + "_Data" + var;
  const std::regex find(anaName + R"(__(.*))" + var);

  auto mcStack = select(*mcFile, find);
  auto dataHist = dynamic_cast<TH1D*>(dataFile->Get(dataName.c_str()));
  if(!dataHist)
  {
    std::cerr << "Failed to find a histogram named " << dataName << "\n";
    return 1;
  }

  std::vector<TFile*> otherMCFiles = {giveMeFileOrGiveMeDeath(otherMCFileNames)...};
  std::vector<THStack> otherMCStacks;
  for(const auto& file: otherMCFiles)
  {
    auto stack = select(*file, find);
    stack.SetName(file->GetName());
    otherMCStacks.push_back(stack);
  }

  //Set histogram styles
  applyColors(*mcStack.GetHists(), MnvColors::GetColors(MnvColors::kOkabeItoDarkPalette));

  //Set up a Canvas split in 2.  The bottom canvas really overlaps the top by margin
  //to avoid drawing the x axis twice.
  TCanvas overall(("Data/MC for " + var).c_str());

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

  mcTotal->GetYaxis()->SetTitle("candidates / event"); //dataHist->GetYaxis()->GetTitle()); //TODO: I had the axes backwards in the original plo
  mcTotal->GetYaxis()->SetLabelSize(labelSize * (bottomFraction + margin));
  mcTotal->GetYaxis()->SetTitleSize(0.06); //TODO: How to guess what these are?
  mcTotal->GetYaxis()->SetTitleOffset(0.6);

  mcTotal->SetLineColor(kRed);
  mcTotal->SetFillColorAlpha(kPink + 1, 0.4);
  mcTotal->SetMaximum(maxMC);
  mcTotal->Draw("E2"); //Draw the error bars

  mcStack.Draw("HISTnostackSAME");
  auto axes = mcStack.GetHistogram(); //N.B.: GetHistogram() would have returned nullptr had I called it before Draw()!

  dataHist->SetLineColor(1);
  dataHist->SetLineWidth(lineSize);
  dataHist->SetMarkerStyle(20); //Resizeable closed circle
  dataHist->SetMarkerColor(1);
  dataHist->SetMarkerSize(0.7);
  dataHist->SetTitle("Data");
  dataHist->Draw("X0SAME");

  auto topLegend = top.BuildLegend(0.5, 0.4, 0.9, 0.9);

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
       mcRatio = static_cast<PlotUtils::MnvH1D*>(mcStack.GetStack()->Last()->Clone());
  ratio->Divide(ratio, mcRatio);

  ratio->SetTitle("data");
  ratio->SetLineWidth(lineSize);
  ratio->SetTitleSize(0);

  ratio->GetYaxis()->SetTitle("Data / MC");
  ratio->GetYaxis()->SetLabelSize(labelSize);
  ratio->GetYaxis()->SetTitleSize(0.16);
  ratio->GetYaxis()->SetTitleOffset(0.2);
  ratio->GetYaxis()->SetNdivisions(505); //5 minor divisions between 5 major divisions

  ratio->GetXaxis()->SetTitleSize(0.16);
  ratio->GetXaxis()->SetTitleOffset(0.9);
  ratio->GetXaxis()->SetTitle("energy deposits [MeV]"); //dataHist->GetXaxis()->GetTitle()); //TODO: I had the axes backwards in the original plot
  ratio->GetXaxis()->SetLabelSize(labelSize);

  ratio->SetMinimum(minRatio);
  ratio->SetMaximum(maxRatio);
  ratio->Draw("E0X0");

  //Draw other models to compare to
  int whichLineStyle = 1;
  const std::string baseFileName = mcFileName.substr(0, mcFileName.find(".root"));
  for(auto& otherModel: otherMCStacks)
  {
    auto modelRatio = static_cast</*PlotUtils::MnvH1D**/TH1D*>(otherModel.GetStack()->Last()->Clone());

    std::string legendName = otherModel.GetName();
    legendName = legendName.substr(legendName.find(baseFileName) + baseFileName.length() + 1, std::string::npos); //+1 for the "_"
    legendName = legendName.substr(0, legendName.find(".root"));
    modelRatio->SetTitle(legendName.c_str());
    modelRatio->SetLineStyle(whichLineStyle++);
    modelRatio->SetLineColor(kBlack);
    modelRatio->SetLineWidth(lineSize);

    modelRatio->Divide(modelRatio, mcRatio);

    modelRatio->SetMinimum(minRatio);
    modelRatio->SetMaximum(maxRatio);
    modelRatio->Draw("HIST SAME");
  }

  auto bottomLegend = bottom.BuildLegend(0.1, 0.6, 0.4, 0.95);

  //Now fill mcRatio with 1 for bin content and fractional error
  for(int whichBin = 0; whichBin <= mcRatio->GetXaxis()->GetNbins(); ++whichBin)
  {
    mcRatio->SetBinError(whichBin, mcRatio->GetBinError(whichBin)/mcRatio->GetBinContent(whichBin));
    mcRatio->SetBinContent(whichBin, 1);
  }

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

  overall.Print((var + "DataMCRatio.png").c_str()); //TODO: Include file name here

  return 0;
}
