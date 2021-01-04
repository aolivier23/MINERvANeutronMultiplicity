#One CMakeLists.txt to rule them all.  One CMakeLists.txt to find them.  One CMakeLists.txt to bring them all and in ${BUILD_TYPE} bind them.
cmake_minimum_required(VERSION 2.8.12)

project(MINERvANeutronMultiplicity)

include(ExternalProject)

#TODO: Don't delete SOURCE_DIR's contents.  Let me manage it manually or cvs update -A if I have to.
#MINERvA dependencies.  They use HEAD of a Fermilab CVS repository for version control at the moment.
#You'll need a kerberos ticket to check them out.  One day, I'd like them to use git.
ExternalProject_Add(PlotUtils
                    CVS_REPOSITORY minervacvs@cdcvs.fnal.gov:/cvs/mnvsoft
                    CVS_MODULE Ana/PlotUtils
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ana/PlotUtils
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})

ExternalProject_Add(UnfoldUtils
                    CVS_REPOSITORY minervacvs@cdcvs.fnal.gov:/cvs/mnvsoft
                    CVS_MODULE Ana/UnfoldUtils
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/Ana/UnfoldUtils
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
                    DEPENDS PlotUtils)

#MParamFiles doesn't need to be built or installed.  It's a bunch of text and .root files that programs need to be able to find.
#Nothing depends on MParamFiles to build, but it does need to be installed before much of libPlotUtils can be used.
#TODO: Do I want to install different tags of MParamFiles?  Set an INSTALL_COMMAND for that.
ExternalProject_Add(MParamFiles
                    CVS_REPOSITORY minervacvs@cdcvs.fnal.gov:/cvs/mnvsoft
                    CVS_MODULE MParamFiles
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/MParamFiles
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND ""
                    INSTALL_COMMAND ""
                    )

#Put MParamFiles into the directory structure
install(DIRECTORY MParamFiles DESTINATION etc)

#Third-party dependencies.
#TODO: Make my own tarball of this as a backup plan?  Think of the nightmare that was v22r1p1 :(
ExternalProject_Add(yaml-cpp
                    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
                    GIT_TAG yaml-cpp-0.6.0
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/yaml-cpp
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})

#The main project for my thesis.  Everything else is a dependency.
#This project is NOT auto-updated once you install it because I often want to develop it locally.
ExternalProject_Add(NucCCNeutrons
                    GIT_REPOSITORY https://github.com/MinervaExpt/NucCCNeutrons.git
                    GIT_TAG develop
                    SOURCE_DIR ${CMAKE_SOURCE_DIR}/NucCCNeutrons
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX} -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} -Dyaml-cpp_DIR:STRING=${CMAKE_INSTALL_PREFIX}/lib/yaml-cpp/cmake/yaml-cpp
                    UPDATE_COMMAND ""
                    PATCH_COMMAND ""
                    DEPENDS PlotUtils UnfoldUtils yaml-cpp)

#TODO: mv other setup.sh scripts to setup_PACKAGE.sh?
#Central setup.sh that knows about other packages.
configure_file(setup.sh.in setup.sh @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/setup.sh DESTINATION bin)
install(FILES setupOnGPVMs.sh DESTINATION bin)
