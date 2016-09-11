# - Find libyaml libraries
# This module finds libyaml if it is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  LIBYAML_FOUND               - have the libyaml libs been found
#  LIBYAML_LIBRARIES           - path to the libyaml library
#  LIBYAML_INCLUDE_DIR         - path to where yaml.h is found

FIND_LIBRARY(LIBYAML_LIBRARY
  NAMES libyaml yaml
  PATHS
    ${LIBYAML_ROOT}
    $ENV{LIBYAML_ROOT}
    ${LIBYAML_LIBRARIES}
  PATH_SUFFIXES
    lib lib/${CMAKE_LIBRARY_ARCHITECTURE}
)

FIND_PATH(LIBYAML_INCLUDE_DIR
  NAMES yaml.h
  PATHS
    ${LIBYAML_ROOT}
    $ENV{LIBYAML_ROOT}
    ${LIBYAML_INCLUDE_DIRS}
    ${LIBYAML_INCLUDE_DIR}
  PATH_SUFFIXES
    include
)

MARK_AS_ADVANCED(
  LIBYAML_LIBRARY
  LIBYAML_INCLUDE_DIR
)
SET(LIBYAML_INCLUDE_DIRS "${LIBYAML_INCLUDE_DIR}")
SET(LIBYAML_LIBRARIES "${LIBYAML_LIBRARY}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibYAML DEFAULT_MSG LIBYAML_LIBRARIES LIBYAML_INCLUDE_DIRS)
