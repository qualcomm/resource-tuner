cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_STANDARD_REQUIRED True)

project("URM")

include(GNUInstallDirs)

# Dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)

# Enable Debug. should be removed in production.
# set(CMAKE_BUILD_TYPE Debug)

# Always build shared libs
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

add_subdirectory(${CMAKE_SOURCE_DIR}/resource-tuner)
