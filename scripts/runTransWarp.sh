#!/bin/bash
MIGRATION_FILE=../NeutronMultiplicity_Tracker_EAvailableBackgroundMC.root
TRUE_HIST=Tracker_Neutron_Multiplicity_EfficiencyNumerator
WARPED_FILE=$1 #NeutronMultiplicity_Tracker_no2p2hEnhancementMC.root #../NeutronMultiplicity_Tracker_EAvailableBackground_noWeightsMC.root
RECO_HIST=Tracker_Neutron_Multiplicity_SelectedMCEvents

OUTFILE_NAME=$(basename $1)

TransWarpExtraction --output_file Warping_$OUTFILE_NAME --data $RECO_HIST --data_file $WARPED_FILE --data_truth $TRUE_HIST --data_truth_file $WARPED_FILE --migration Tracker_Neutron_Multiplicity_Migration --migration_file $MIGRATION_FILE --reco $RECO_HIST --reco_file $MIGRATION_FILE --truth $TRUE_HIST --truth_file $MIGRATION_FILE --num_iter 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,40,50,60,70,80,90,100 --num_uni 100
