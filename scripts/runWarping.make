#Run a warping study over several playlists in parallel.  Manages threads using GNU make.
#Designed for a bash shell.
#--ignore-errors helps if madd is crashing at the end of the job.
#USAGE: ANALYSIS=someFile.yaml make -f runWarping.make -j nproc

TUPLE_DIR="/media/anaTuples/validateSL7"
PLAYLISTS:= $(shell ls $(TUPLE_DIR))
CV_NAME:=$(shell basename $(ANALYSIS) .yaml)_cv
CV_FILES:=$(foreach DIR,$(PLAYLISTS),$(DIR)/$(CV_NAME)MC.root)
WARPED_NAME:=$(shell basename $(ANALYSIS) .yaml)_warped
WARPED_FILES:=$(foreach DIR,$(PLAYLISTS),$(DIR)/$(WARPED_NAME)MC.root)

#TransWarpExtraction study configuration
N_STAT_UNIVS:=100
MIGRATION_FILE:=$(CV_NAME)MC.root
ITER_TO_TEST:= 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,40,50,60,70,80,90,100
RECO_HIST:=Tracker_Neutron_Multiplicity_SelectedMCEvents
TRUE_HIST:=Tracker_Neutron_Multiplicity_EfficiencyNumerator

results/combined.csv: results
	cat results/Warping*.csv > results/combined.csv

results: transWarp
	mkdir -p results && cd results $(foreach STUDY,$(wildcard transWarp/*.root),&& root -l -b -q '~/app/MINERvANeutronMultiplicity/src/scripts/warpingTable.cpp("../$(STUDY)")')

transWarp: warps merged/$(MIGRATION_FILE)
	mkdir -p transWarp $(foreach WARPED_FILE,$(wildcard warps/$(WARPED_NAME)MC_*.root),&& TransWarpExtraction --output_file transWarp/Warping_$(shell basename $(WARPED_FILE) .root).root --data $(RECO_HIST) --data_file $(WARPED_FILE) --data_truth $(TRUE_HIST) --data_truth_file $(WARPED_FILE) --migration Tracker_Neutron_Multiplicity_Migration --migration_file merged/$(MIGRATION_FILE) --reco $(RECO_HIST) --reco_file merged/$(MIGRATION_FILE) --truth $(TRUE_HIST) --truth_file merged/$(MIGRATION_FILE) --num_iter $(ITER_TO_TEST) --num_uni $(N_STAT_UNIVS))

warps: merged/$(WARPED_NAME)MC.root
	mkdir -p warps && cd warps && SwapSysUnivWithCV ../$^

merged/$(MIGRATION_FILE): $(CV_FILES)
	mkdir -p merged && madd $@ $^

merged/$(WARPED_NAME)MC.root: $(WARPED_FILES)
	mkdir -p merged && madd $@ $^

%/$(CV_NAME)MC.root: %/$(CV_NAME).yaml
	cd $* && ProcessAnaTuples $(CV_NAME).yaml $(TUPLE_DIR)/$*/mc/*.root

%/$(WARPED_NAME)MC.root: %/$(WARPED_NAME).yaml
	cd $* && ProcessAnaTuples $(WARPED_NAME).yaml $(TUPLE_DIR)/$*/mc/*.root

%/$(WARPED_NAME).yaml:
	mkdir -p $* && cd $* && cat ../Warps.yaml ../$(ANALYSIS) > $(WARPED_NAME).yaml

%/$(CV_NAME).yaml:
	mkdir -p $* && cd $* && cat ../Systematics.yaml ../$(ANALYSIS) > $(CV_NAME).yaml

#TODO: Do I want to delete the results in the transWarp directory?  Choosing not to right now.
.PHONY: clean
clean:
	rm -r $(PLAYLISTS); rm -r merged; rm -r warps
