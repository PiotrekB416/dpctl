cmake_minimum_required(VERSION 3.21)

project(external_usm_allocation LANGUAGES CXX)

set(DPCTL_CMAKE_MODULES_PATH "${CMAKE_SOURCE_DIR}/../../../cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${DPCTL_CMAKE_MODULES_PATH})
find_package(IntelDPCPP REQUIRED PATHS ${DPCTL_CMAKE_MODULES_PATH} NO_DEFAULT_PATH)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Fetch pybind11
include(FetchContent)
FetchContent_Declare(
  pybind11
  URL https://github.com/pybind/pybind11/archive/refs/tags/v2.10.2.tar.gz
  URL_HASH SHA256=93bd1e625e43e03028a3ea7389bba5d3f9f2596abc074b068e70f4ef9b1314ae
)
FetchContent_MakeAvailable(pybind11)

find_package(PythonExtensions REQUIRED)
find_package(Dpctl REQUIRED)
find_package(NumPy REQUIRED)

set(py_module_name _external_usm_alloc)
pybind11_add_module(${py_module_name}
    MODULE
    external_usm_allocation/_usm_alloc_example.cpp
)
target_include_directories(${py_module_name} PUBLIC ${Dpctl_INCLUDE_DIRS})
install(TARGETS ${py_module_name}
  DESTINATION external_usm_allocation
)

set(ignoreMe "${SKBUILD}")
