configure_file(runWarping.sh.in runWarping.sh @ONLY)

#Actual executables
install(FILES runWarping.make ${CMAKE_CURRENT_BINARY_DIR}/runWarping.sh runTransWarp.sh DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

#Macros.  They go to bin right now, but I might put them somewhere else one day.
install(FILES backgroundBreakdown.cpp candOrigins.yaml compareErrorBands.yaml dataMCRatio.cpp edepsWithRatioFromLEPaper.cpp getFiles.sh migration.yaml plotSideband.cpp plotUncertaintySummary.cpp selectionEfficiency.yaml smearingFractionStudy.cpp warpingTable.cpp plotEfficiencyAndProcesses.cpp DESTINATION bin)
