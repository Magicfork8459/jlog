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
    add_executable(jlog_test_file_logs
        ${PROJECT_SOURCE_DIR}/tests/file_logs.cpp
        ${PROJECT_SOURCE_DIR}/tests/fixture.hpp
    )

    target_include_directories(jlog_test_file_logs PUBLIC
        ${Boost_INCLUDE_DIRS}   
        ${PROJECT_SOURCE_DIR}/src     
    )

    target_link_libraries(jlog_test_file_logs
        ${Boost_LIBRARIES}
        ${PROJECT_NAME}
    )

    set_target_properties(jlog_test_file_logs PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 23
    )

    add_executable(jlog_test_constructors
        ${PROJECT_SOURCE_DIR}/tests/constructors.cpp
    )

    target_include_directories(jlog_test_constructors PUBLIC
        ${Boost_INCLUDE_DIRS}   
        ${PROJECT_SOURCE_DIR}/src     
    )

    target_link_libraries(jlog_test_constructors
        ${Boost_LIBRARIES}
        ${PROJECT_NAME}
    )

    set_target_properties(jlog_test_constructors PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 23
    )

    #TODO make this optional
    enable_testing()

    add_test(NAME "file_logs" COMMAND jlog_test_file_logs)
    add_test(NAME "constructors" COMMAND jlog_test_constructors)