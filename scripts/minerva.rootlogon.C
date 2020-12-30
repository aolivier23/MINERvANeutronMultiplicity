{
  if( gSystem->Getenv("PLOTUTILSROOT") )
  {
    gInterpreter->AddIncludePath( gSystem->ExpandPathName("$PLOTUTILSROOT") );
    string newpath = string(gROOT->GetMacroPath()) + ":" + string(gSystem->ExpandPathName("$PLOTUTILSROOT")) + "/PlotUtils";
    //gROOT->SetMacroPath( newpath.c_str() );
    
    gSystem->Load( gSystem->ExpandPathName("$PLOTUTILSROOT/libPlotUtils.so") );
    std::cout << "tried to load libPlotUtils.so" << endl;
    //gInterpreter->ExecuteMacro("PlotStyle.C");
  }

}

