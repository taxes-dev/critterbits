cmake_minimum_required(VERSION 3.3)
project(critterbits)

# Use our modified FindSDL2* modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${critterbits_SOURCE_DIR}/cmake")

# Set an output directory for our binaries
set(BIN_DIR ${critterbits_SOURCE_DIR}/bin)

# Third party dependencies
set(PROJECT_TRDPARTY_DIR ${critterbits_SOURCE_DIR}/3pp)
set(TINYXML2_ROOT ${PROJECT_TRDPARTY_DIR}/tinyxml2-4.0.1)
set(TMXPARSER_ROOT ${PROJECT_TRDPARTY_DIR}/tmxparser)
set(ZSTD_ROOT ${PROJECT_TRDPARTY_DIR}/zstd)

# Bump up warning levels appropriately for clang, gcc & msvc
# Also set debug/optimization flags depending on the build type. IDE users choose this when
# selecting the build mode in their IDE
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-switch -Wno-unused-parameter -pedantic -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(MSVC_IGNORE_WARNINGS "/wd4456 /wd4267 /wd4244")
	if(CMAKE_GENERATOR MATCHES "Win64")
		set(CMAKE_LIBRARY_ARCHITECTURE "win32_x64")
	else()
		set(CMAKE_LIBRARY_ARCHITECTURE "win32_x86")
	endif()
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_IGNORE_WARNINGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 ${MSVC_IGNORE_WARNINGS}")
	endif()
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=c99")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -Os -fomit-frame-pointer")
elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
endif()

# Look up SDL2 and add the include directory to our include path
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_gfx REQUIRED)

# Find TMX parser dependencies (tmxparser, tinyxml2)
find_package(TINYXML2 REQUIRED)
find_package(TMXPARSER REQUIRED)

# Zstandard dependencies
find_package(ZSTD REQUIRED)

include_directories(SYSTEM ${SDL2_INCLUDE_DIR} ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_GFX_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR}
	${TINYXML2_INCLUDE_DIRS}
	${ZSTD_INCLUDE_DIRS}
	${TMXPARSER_INCLUDE_DIRS} 3pp)
include_directories(include)

# clang-format target
file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.hpp)
foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
    string(FIND ${SOURCE_FILE} ${PROJECT_TRDPARTY_DIR} PROJECT_TRDPARTY_DIR_FOUND)
    if (NOT ${PROJECT_TRDPARTY_DIR_FOUND} EQUAL -1)
        list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
    endif()
endforeach()
add_custom_target(clangformat COMMAND /usr/bin/clang-format -style=file -i ${ALL_SOURCE_FILES} )

# Look in the subdirectories to find additional CMakeLists.txt so we can build executables
add_subdirectory(src)
add_subdirectory(3pp/duktape)
