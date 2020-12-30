//File: smearingFractionStudy.cpp
//Brief: Given a migration matrix in a variable I want to cut on, graph the fraction of events that pass both
//       the reconstructed and the truth cut for different cut values.   This helps me decide whether a cut
//       will need background subtraction or will be sufficient as a phase space cut.  Values less than 90%
//       or so probably need background subtraction.
//Author: Andrew Olivier aolivier@ur.rochester.edu
//Usage: root -l smearingFractionStudy.cpp+g("nameOfFile.root", "nameOfMigrationMatrix")
//N.B.: X axis is asummed to be TRUTH
//TODO: Figure that out from axis labels

//ROOT includes
#include "TFile.h"
#include "TH2.h"
#include "TGraph.h"
#include "TCanvas.h"

//c++ includes
#include <iostream>
#include <memory>

int smearingFractionStudy(const std::string& fileName, const std::string& histName)
{
  auto file = TFile::Open(fileName.c_str());
  if(!file)
  {
    std::cerr << "Failed to open " << fileName << ".  Bailing...\n";
    return 1;
  }

  auto obj = file->Get(histName.c_str());
  if(!obj)
  {
    std::cerr << "Failed to find a TH2 named " << histName << " in " << fileName << ".  Bailing...\n";
    return 2;
  }

  auto hist = dynamic_cast<TH2*>(obj);
  if(!hist)
  {
    std::cerr << "Found a TObject named " << histName << " in " << fileName << ", but it's not a TH2.  "
              << "So I can't project it.  Bailing...\n";
    return 3;
  }

  const int nBins = hist->GetXaxis()->GetNbins();
  assert(nBins == hist->GetYaxis()->GetNbins() && "Number of x bins in migration matrix must be the same as number of y bins!");

  //Figure out which axis has truth quantities to make it harder to mess up this study
  bool xIsTrue = true; //Default assumption in case I can't detect truth
  {
    std::string origXLabel = hist->GetXaxis()->GetTitle(),
                origYLabel = hist->GetYaxis()->GetTitle();
    if((origXLabel.find("True") == std::string::npos))
    {
      if(origYLabel.find("True") != std::string::npos) xIsTrue = false;
      else std::cout << "Failed to find \"True\" in either axis label.  "
                     << "Assuming that the x axis has a truth quantity and the Y axis has a reco quantity.\n";
    }
  }

  auto truthAxis = xIsTrue?hist->GetXaxis():hist->GetYaxis();

  //Set up the graph I'm going to draw
  TGraph graph(nBins);
  graph.SetTitle("Smearing Fraction Study");

  std::string newAxisLabel = truthAxis->GetTitle();
  const size_t foundTrue = newAxisLabel.find("True");
  if(foundTrue != std::string::npos) newAxisLabel.erase(foundTrue, foundTrue + 5);
  newAxisLabel += " Cut";

  graph.GetXaxis()->SetTitle(newAxisLabel.c_str()); //TODO: Remove "True" from axis label?
  graph.GetYaxis()->SetTitle("% of Reco Events that Pass Reco and True Cuts");

  //Keep track of axis limits to zoom in on region of interest
  double minY = 100, maxY = 0;

  for(int whichBin = 0; whichBin < nBins; ++whichBin)
  {
    //First, cut along reco axis by projecting into the truth axis
    std::unique_ptr<TH1D> proj(xIsTrue?hist->ProjectionX("TemporaryProjection", 0, whichBin):hist->ProjectionY("TemporaryProjection", 0, whichBin));

    if(proj->Integral() > 0)
    {
      //Now, graph the percentage of reco events that passed both cuts.
      const double percentPassed = proj->Integral(0, whichBin)/proj->Integral()*100.;
      minY = std::min(percentPassed, minY);
      maxY = std::max(percentPassed, maxY);
      graph.SetPoint(whichBin, truthAxis->GetBinUpEdge(whichBin), percentPassed);
    }
  }

  TCanvas canvas("smearingFractionStudy");
  graph.SetMinimum(minY - 5);
  graph.SetMaximum(std::min(maxY + 5, 100.));
  graph.SetMarkerStyle(kFullCircle);
  graph.Draw("AP");
  canvas.Print((histName + "_smearingFractionStudy.png").c_str());

  return 0;
}
