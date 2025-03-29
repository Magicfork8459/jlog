cmake_minimum_required(VERSION 3.29.2)

set(TEST_NAME_FILES jlog_test_file_logs)
set(TEST_NAME_CONSTRUCTORS jlog_test_constructors)

add_executable(${TEST_NAME_FILES}
    file_logs.cpp
    fixture.hpp
)

target_include_directories(${TEST_NAME_FILES} PUBLIC
    ${Boost_INCLUDE_DIRS}   
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(${TEST_NAME_FILES}
    ${Boost_LIBRARIES}
    ${PROJECT_NAME}
)

set_target_properties(${TEST_NAME_FILES} PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 23
)

add_executable(${TEST_NAME_CONSTRUCTORS}
    constructors.cpp
)

target_include_directories(${TEST_NAME_CONSTRUCTORS} PUBLIC
    ${Boost_INCLUDE_DIRS}   
    ${PROJECT_SOURCE_DIR}   
)

target_link_libraries(${TEST_NAME_CONSTRUCTORS}
    ${Boost_LIBRARIES}
    ${PROJECT_NAME}
)

set_target_properties(${TEST_NAME_CONSTRUCTORS} PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 23
)

add_test(NAME "file_logs" COMMAND ${TEST_NAME_FILES})
add_test(NAME "constructors" COMMAND ${TEST_NAME_CONSTRUCTORS})