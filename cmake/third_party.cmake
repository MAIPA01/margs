# for CPM
set(CPM_USE_LOCAL_PACKAGES OFF)

if(NOT CPM_SOURCE_CACHE)
    # Storage location
    set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/third_party)
endif()

# Set download location
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")

# download CPM.cmake
# Expand relative path. This is important if the provided path contains a tilde (~)
get_filename_component(CPM_DOWNLOAD_LOCATION ${CPM_DOWNLOAD_LOCATION} ABSOLUTE)
if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
	file(DOWNLOAD
		https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
	)
endif()
message(STATUS "Include CPM.cmake from ${CPM_DOWNLOAD_LOCATION}")
include(${CPM_DOWNLOAD_LOCATION})

# Add mstd library
CPMAddPackage("gh:MAIPA01/mstd#main")

if(mstd_ADDED)
    include_directories(${mstd_SOURCE_DIR}/include)

    message(STATUS "CMake added local mstd: ${mstd_SOURCE_DIR}")
endif()