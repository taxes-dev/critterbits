cmake_minimum_required(VERSION 2.6)
project(critterbits)

# Use our modified FindSDL2* modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${critterbits_SOURCE_DIR}/cmake")

# Set an output directory for our binaries
set(BIN_DIR ${critterbits_SOURCE_DIR}/bin)

# Bump up warning levels appropriately for clang, gcc & msvc
# Also set debug/optimization flags depending on the build type. IDE users choose this when
# selecting the build mode in their IDE
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-switch -Wno-unused-parameter -pedantic -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
endif()

if (${CMAKE_C_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -Os -fomit-frame-pointer")
elseif (${CMAKE_C_COMPILER_ID} STREQUAL "MSVC")
endif()

# Look up SDL2 and add the include directory to our include path
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_gfx REQUIRED)

# Look up libyaml and add dependencies
find_package(YAML REQUIRED)

# Look up zlib and libxml2 (used by TMX)
find_package(ZLIB REQUIRED)
find_package(LibXml2 REQUIRED)

# TMX library
find_library(LIBTMX_LIBRARY NAMES libtmx tmx PATHS 3pp/tmx NO_DEFAULT_PATH)

include_directories(${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR} ${SDL2_GFX_INCLUDE_DIR} ${LIBYAML_INCLUDE_DIR} 3pp include)

# clang-format target
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h)
add_custom_target(clangformat COMMAND /usr/bin/clang-format -style=file -i ${ALL_SOURCE_FILES} )

# Look in the subdirectories to find additional CMakeLists.txt so we can build executables
add_subdirectory(src)
add_subdirectory(3pp/duktape)
