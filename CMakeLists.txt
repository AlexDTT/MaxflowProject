cmake_minimum_required(VERSION 3.26)
project(MaxFlowProject)

set(CMAKE_CXX_STANDARD 14)
set( CMAKE_BUILD_TYPE_TMP "${CMAKE_BUILD_TYPE}" )
set( CMAKE_BUILD_TYPE "Debug" )

set( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_TMP} )

add_executable(main
    main.cpp
    utils/Submission.cpp
    utils/Reviewer.cpp
    utils/FileParser.cpp
    utils/EdmondKarp.cpp
    data_structures/createGraphs.cpp
)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})


