cmake_minimum_required(VERSION 3.26)
project(MaxFlowProject)

set(CMAKE_CXX_STANDARD 17)
set( CMAKE_BUILD_TYPE_TMP "${CMAKE_BUILD_TYPE}" )
set( CMAKE_BUILD_TYPE "Debug" )

set( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_TMP} )

add_executable(main
    main.cpp
    ui/ConferenceApp.cpp
    models/Submission.cpp
    models/Reviewer.cpp
    io/FileParser.cpp
    algorithms/EdmondKarp.cpp
    services/AssignmentLogic.cpp
    data_structures/GraphBuilder.cpp
)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})

enable_testing()

add_executable(maxflow_tests
    tests/maxflow_tests.cpp
    models/Submission.cpp
    models/Reviewer.cpp
    algorithms/EdmondKarp.cpp
    services/AssignmentLogic.cpp
    data_structures/GraphBuilder.cpp
)

target_include_directories(maxflow_tests PRIVATE ${CMAKE_SOURCE_DIR})

add_test(NAME unit_tests COMMAND maxflow_tests)

add_test(NAME integration_tests
    COMMAND bash ${CMAKE_SOURCE_DIR}/tests/run_integration_tests.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Doxygen target
find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()


