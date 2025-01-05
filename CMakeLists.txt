cmake_minimum_required(VERSION 3.29.2)

project(jlog)

find_package(Boost REQUIRED COMPONENTS log log_setup unit_test_framework json)

# Setup Library
    file(GLOB SOURCE
        ${PROJECT_SOURCE_DIR}/src/*.cpp
        ${PROJECT_SOURCE_DIR}/src/*.h
    )

    add_library(${PROJECT_NAME} ${SOURCE})

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${Boost_INCLUDE_DIRS}        
    )

    target_link_libraries(${PROJECT_NAME}
        ${Boost_LIBRARIES}
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 23
    )

# Setup Tests
    set(TEST_PROJECT_NAME test_${PROJECT_NAME})

    add_executable(${TEST_PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/tests/test_logger.cpp
    )

    target_include_directories(${TEST_PROJECT_NAME} PUBLIC
        ${Boost_INCLUDE_DIRS}   
        ${PROJECT_SOURCE_DIR}/src     
    )

    target_link_libraries(${TEST_PROJECT_NAME}
        ${Boost_LIBRARIES}
        ${PROJECT_NAME}
    )

    set_target_properties(${TEST_PROJECT_NAME} PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 23
    )

    #TODO make this optional
    enable_testing()

    add_test(NAME "JLog Tests" COMMAND ${TEST_PROJECT_NAME})