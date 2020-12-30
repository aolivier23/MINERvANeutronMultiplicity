#!/usr/bin/env bash
PATH_ON_GPVM=/minerva/data/users/aolivier/NucCCNeutron/systematics/selectionEfficiency/me6A/mc/afterTruthEAvailableBugFix
BASE_NAME=NeutronMultiplicityTracker_

scp aolivier@minervagpvm03.fnal.gov:${PATH_ON_GPVM}/${BASE_NAME}*.root .
scp aolivier@minervagpvm03.fnal.gov:${PATH_ON_GPVM}/EAvailableResolution*.root .
