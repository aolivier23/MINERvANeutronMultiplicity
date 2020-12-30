# MINERvANeutronMultiplicity

## Description

Installation and development workflow for [NucCCNeutrons](https://github.com/MinervaExpt/NucCCNeutrons) and its dependencies.  Leverages CMake's ExternalProject package to manage dependencies automatically.  Also includes some scripts for working with the output of NucCCNeutrons.

##Installation

1. `mkdir MINERvANeutronMultiplicity && cd MINERvANeutronMultiplicity`
1. `git clone` this repository into `src`
2. `mkdir opt && cd opt && mkdir build && cd build #Make a location for an out of source build.`
3. `cmake ../../src -DCMAKE_INSTALL_PREFIX=`pwd`/.. -DCMAKE_BUILD_TYPE=Release #Generate installation instructions with CMake`
4. Get a Fermilab Kerberos ticket.  Needed to download PlotUtils and UnfoldUtils as long as they come from the MINERvA CVS repository.
5. `make install #Also checks out source code for dependencies.  Add -j 8 to use 8 cores for example.`

##Usage

1. Make sure ROOT is set up.  I usually do this much automatically in my .bashrc.
2. `source /path/to/MINERvANeutronMultiplicity/opt/bin/setup.sh` from the installation example to get the libraries you need for ProcessAnaTuples.
3. Edit source code directly in the `src` directory.  `make [all]` cloned it for you.  Each package is version-controlled separately and should be ignored by this package's git repository.  So, you have to `cd src/NucCCNeutrons` before you can commit a change to `src/NucCCNeutrons/evt/Universe.h`.
4. To rebuild changes in *any* package, `cd /path/to/MINERvANeutronMultiplicity/opt/build && make install -j 8`.  *This will merge with the latest PlotUtils and UnfoldUtils by default*.

##Tricks and Gotchas

- Most of the executables and interesting configuration files are in NucCCNeutrons.  They'll all end up in `opt/bin` of course.  PlotUtils, UnfoldUtils, and yaml-cpp just provide libraries.
- `make install` will merge CVS repositories with their remotes by default.  You can change this by setting an empty `UPDATE_COMMAND` in CMakeLists.txt for this packages.  See CMake's [ExternalProject documentation](https://cmake.org/cmake/help/latest/module/ExternalProject.html) for details.  I disabled automatic updates for NucCCNeutrons because *git repositories default to overwriting your changes!*.  I strongly recommend you do the same for any git repositories you add.
- You don't have to call your install type `opt`.  One advantage of this workflow (and out of source builds in general) is that I can have multiple build types.  These packages are set up to support 3 build types as of writing: opt(imizied), debug, and prof(iling).  So, I like to replace `opt` with `debug` in the Installation instructions and pass `-DCMAKE_BUILD_TYPE=Debug` to create a build of the entire project that provides maximal information for gdb and valgrind.
- If you add anything that depends on one of these ExternalProjects, it must also be an ExternalProject in its own repository.  As of CMake 2.8.12, the lowest common denominator with Scientific Linux 7, it's very hard to have a regular target depend on an ExternalProject.  I'm letting ROOT be an exception to this rule because I have never needed to develop it in parallel with my analysis.
- Please send questions and report bugs to Andrew Olivier at the University of Rochester
