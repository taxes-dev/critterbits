# - Find zstd libraries
# This module finds zstd if it is installed and determines where the
# include files and libraries are. It also determines what the name of
# the library is. This code sets the following variables:
#
#  ZSTD_FOUND               - have the zstd libs been found
#  ZSTD_LIBRARIES           - path to the zstd library
#  ZSTD_INCLUDE_DIR         - path to where zstd.h is found
#
# Set ZSTD_ROOT to give a hint where the libraries are installed

FIND_LIBRARY(ZSTD_LIBRARY
  NAMES libzstd zstd zstdlib_x64 zstdlib_x86
  PATHS
    ${ZSTD_ROOT}
    $ENV{ZSTD_ROOT}
  NO_DEFAULT_PATHS
  PATH_SUFFIXES
    lib lib/${CMAKE_LIBRARY_ARCHITECTURE}
)

FIND_PATH(ZSTD_INCLUDE_DIR
  NAMES zstd.h
  PATHS
    ${ZSTD_ROOT}
    $ENV{ZSTD_ROOT}
  PATH_SUFFIXES
    include
)

MARK_AS_ADVANCED(
  ZSTD_LIBRARY
  ZSTD_INCLUDE_DIR
)
SET(ZSTD_INCLUDE_DIRS "${ZSTD_INCLUDE_DIR}")
SET(ZSTD_LIBRARIES "${ZSTD_LIBRARY}")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZSTD DEFAULT_MSG ZSTD_LIBRARIES ZSTD_INCLUDE_DIRS)
