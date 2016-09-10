# - Find tinyxml2 libraries
# This module finds tinyxml2 if it is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  TINYXML2_FOUND               - have the tinyxml2 libs been found
#  TINYXML2_LIBRARIES           - path to the tinyxml2 library
#  TINYXML2_INCLUDE_DIR         - path to where tiyxml2.h is found
#
# Set TINYXML2_ROOT to give a hint where the libraries are installed

FIND_LIBRARY(TINYXML2_LIBRARY
  NAMES libtinyxml2 tinyxml2
  PATHS
    ${TINYXML2_ROOT}
    $ENV{TINYXML2_ROOT}
  NO_DEFAULT_PATHS
  PATH_SUFFIXES
    lib lib/${CMAKE_LIBRARY_ARCHITECTURE}
)

FIND_PATH(TINYXML2_INCLUDE_DIR
  NAMES tinyxml2.h
  PATHS
    ${TINYXML2_ROOT}
    $ENV{TINYXML2_ROOT}
  PATH_SUFFIXES
    include
)

MARK_AS_ADVANCED(
  TINYXML2_LIBRARY
  TINYXML2_INCLUDE_DIR
)
SET(TINYXML2_INCLUDE_DIRS "${TINYXML2_INCLUDE_DIR}")
SET(TINYXML2_LIBRARIES "${TINYXML2_LIBRARY}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TINYXML2 DEFAULT_MSG TINYXML2_LIBRARIES TINYXML2_INCLUDE_DIRS)
