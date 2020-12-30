#Extra set up to build standalone MINERvA macros with CMake.
#Needed to detect LCG (= LHC Computing Grid?) components that
#I think our ROOT installation was configured to link against.
#Pass -d on the command line to find debug builds.

MINERVA_VERSION="v22r1p1"
CMTCONFIG="x86_64-slc7-gcc49"
MINERVA_INSTALL_DIR="/cvmfs/minerva.opensciencegrid.org/minerva"

LCG_BUILD_TYPE="opt"
if [ "$1" = "-d" ]
then
  echo "Setting up in debug mode"
  LCG_BUILD_TYPE="dbg"
fi

#Make sure I can still commit to PlotUtils via CVS
export CVSROOT=minervacvs@cdcvs.fnal.gov:/cvs/mnvsoft

#Set up MINERvA UPS products
source ${MINERVA_INSTALL_DIR}/setup/setup_minerva_products.sh
setup gcc v4_9_3

#CMake needs these environment variables set to detect the correct version of gcc.
export CXX=`which g++`
export CC=`which gcc`

#LCG (= LHC Computing Grid) components.
#Set up cmake
export PATH=${MINERVA_INSTALL_DIR}/software_releases/${MINERVA_VERSION}/lcg/external/cmake_sl7/bin:${PATH}

#ROOT needs an older version of liblzma that doesn't appear to ship with SL7.
#export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${MINERVA_INSTALL_DIR}/software_releases/grid_libs/lib

#Set up ROOT
source ${MINERVA_INSTALL_DIR}/software_releases/${MINERVA_VERSION}/lcg/external/ROOT/5.34.36/${CMTCONFIG}-${LCG_BUILD_TYPE}/bin/thisroot.sh

#Set up xrootd for TNetXNGFile
export LD_LIBRARY_PATH=${MINERVA_INSTALL_DIR}/software_releases/${MINERVA_VERSION}/lcg/external/xrootd/3.3.6/${CMTCONFIG}-${LCG_BUILD_TYPE}/lib64:${LD_LIBRARY_PATH}

#Set up gccxml for Reflex dictionary generation (needed for CINT and python bindings in PlotUtils)
export PATH=${MINERVA_INSTALL_DIR}/software_releases/${MINERVA_VERSION}/lcg/external/LCGCMT/LCGCMT_61/LCG_Settings/../../../gccxml/0.9.0_20150423/${CMTCONFIG}-${LCG_BUILD_TYPE}/bin:${PATH}

#Set up the GNU Scientific Library.
#Our ROOT installation on the GPVMs seems to be linked against it in some weird way that I don't understand.
export LD_LIBRARY_PATH=${MINERVA_INSTALL_DIR}/software_releases/${MINERVA_VERSION}/lcg/external/GSL/1.10/${CMTCONFIG}-${LCG_BUILD_TYPE}/lib:${LD_LIBRARY_PATH}
