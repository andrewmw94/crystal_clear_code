cmake_minimum_required(VERSION 3.1.0)
project(sat_solver_cpp)

set(LIBRARY_NAME ${PROJECT_NAME})


set(CMAKE_MODULE_PATH
    "${CMAKE_MODULE_PATH}"
    "${CMAKE_ROOT_DIR}/cmake/Modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
  
include(CompilerSettings)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib )


list(APPEND SOURCES
    src/dpll.cpp
    )


include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_executable(sat_solver scripts/main.cpp ${SOURCES})