# - Find tmxparser libraries
# This module finds tmxparser if it is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TMXPARSER_FOUND               - have the tmxparser libs been found
#  TMXPARSER_LIBRARIES           - path to the tmxparser library
#  TMXPARSER_INCLUDE_DIR         - path to where Tmx.h is found
#
# Set TMXPARSER_ROOT to give a hint where the libraries are installed

FIND_LIBRARY(TMXPARSER_LIBRARY
  NAMES libtmxparser tmxparser
  PATHS
    ${TMXPARSER_ROOT}
    $ENV{TMXPARSER_ROOT}
  NO_DEFAULT_PATHS
  PATH_SUFFIXES
    lib lib/${CMAKE_LIBRARY_ARCHITECTURE}
)

FIND_PATH(TMXPARSER_INCLUDE_DIR
  NAMES Tmx.h
  PATHS
    ${TMXPARSER_ROOT}
    $ENV{TMXPARSER_ROOT}
  PATH_SUFFIXES
    include
)

MARK_AS_ADVANCED(
  TMXPARSER_LIBRARY
  TMXPARSER_INCLUDE_DIR
)
SET(TMXPARSER_INCLUDE_DIRS "${TMXPARSER_INCLUDE_DIR}")
SET(TMXPARSER_LIBRARIES "${TMXPARSER_LIBRARY}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TMXPARSER DEFAULT_MSG TMXPARSER_LIBRARIES TMXPARSER_INCLUDE_DIRS)
