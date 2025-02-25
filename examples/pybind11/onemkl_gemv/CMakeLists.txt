cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(example_use_mkl_gemm LANGUAGES CXX)
set(DPCTL_CMAKE_MODULES_PATH "${CMAKE_SOURCE_DIR}/../../../cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${DPCTL_CMAKE_MODULES_PATH})
find_package(IntelDPCPP REQUIRED PATHS ${DPCTL_CMAKE_MODULES_PATH} NO_DEFAULT_PATH)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define CMAKE_INSTALL_xxx: LIBDIR, INCLUDEDIR
include(GNUInstallDirs)

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

find_library(mkl_core NAMES mkl_core PATHS ${MKL_LIBRARY_DIR} REQUIRED)
find_library(mkl_sycl NAMES mkl_sycl PATHS ${MKL_LIBRARY_DIR} REQUIRED)
find_library(mkl_intel_ilp64 NAMES mkl_intel_ilp64 PATHS ${MKL_LIBRARY_DIR} REQUIRED)
find_library(mkl_tbb_thread NAMES mkl_tbb_thread PATHS ${MKL_LIBRARY_DIR} REQUIRED)
find_library(tbb NAMES tbb PATHS ${TBB_LIBRARY_DIR} REQUIRED)
find_library(OpenCL NAMES OpenCL REQUIRED)

set(py_module_name _onemkl)

pybind11_add_module(${py_module_name}
    MODULE
    sycl_gemm/_onemkl.cpp
)
target_compile_definitions(${py_module_name} PRIVATE -DMKL_ILP64)
target_include_directories(${py_module_name}
    PUBLIC ${MKL_INCLUDE_DIR} sycl_gemm
)
target_link_libraries(${py_module_name}
    PRIVATE ${mkl_sycl} ${mkl_intel_ilp64} ${mkl_tbb_thread} ${mkl_core} ${tbb}
)

install(TARGETS ${py_module_name} DESTINATION sycl_gemm)
target_include_directories(${py_module_name} PUBLIC ${Dpctl_INCLUDE_DIRS})

get_target_property(_sycl_gemm_sources ${py_module_name} SOURCES)
set_source_files_properties(${_sycl_gemm_sources}
  PROPERTIES
  COMPILE_OPTIONS "-O3"
)
target_link_options(${py_module_name} PRIVATE -fsycl-device-code-split=per_kernel)

add_executable(standalone_cpp
  EXCLUDE_FROM_ALL
  cpp/main.cpp
)
target_compile_options(standalone_cpp
  PRIVATE -O3 -Wno-deprecated-declarations
)
target_include_directories(standalone_cpp
  PUBLIC ${MKL_INCLUDE_DIR} ${TBB_INCLUDE_DIR} sycl_gemm
  )
target_link_libraries(standalone_cpp
    PRIVATE ${mkl_sycl} ${mkl_intel_ilp64} ${mkl_tbb_thread} ${mkl_core} ${tbb} ${OpenCL}
)

set(ignoreMe "${SKBUILD}")
